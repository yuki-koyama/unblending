#include <unblending/unblending.hpp>
#include <fstream>
#include <iostream>
#include <json11.hpp>
#include <Eigen/LU>

namespace unblending
{
    using std::vector;
    using json11::Json;
    
    namespace
    {
        vector<Json> interpret_mat3_as_json(const Mat3& x)
        {
            vector<Json> tmp;
            for (int col = 0; col < static_cast<int>(x.cols()); ++ col)
            {
                for (int row = 0; row < static_cast<int>(x.rows()); ++ row)
                {
                    tmp.push_back(Json(x(col, row)));
                }
            }
            return tmp;
        }
        
        Json interpret_variance_as_json(const Mat3& sigma)
        {
            // Determine whether the covariance matrix is uniform or not
            assert(sigma(0, 0) != 0.0);
            if ((sigma / sigma(0, 0)).isIdentity())
            {
                return Json(sigma(0, 0));
            }
            else
            {
                return interpret_mat3_as_json(sigma);
            }
        }
        
        Mat3 interpret_json_as_mat3(const Json& json)
        {
            Mat3 mat;
            for (int col = 0; col < 3; ++ col)
            {
                for (int row = 0; row < 3; ++ row)
                {
                    mat.coeffRef(row, col) = json.array_items()[col * 3 + row].number_value();
                }
            }
            return mat;
        }
        
        Vec3 interpret_json_as_vec3(const Json& json)
        {
            return (Vec3() << json.array_items()[0].number_value(), json.array_items()[1].number_value(), json.array_items()[2].number_value()).finished();
        }
        
        vector<Json> interpret_vecx_as_json(const VecX& x)
        {
            vector<Json> tmp;
            for (int i = 0; i < static_cast<int>(x.rows()); ++ i)
            {
                tmp.push_back(Json(x(i)));
            }
            return tmp;
        }
        
        Mat3 interpret_json_as_variance(const Json& json)
        {
            // Determine whether the covariance matrix is uniform or not
            if (json.is_number())
            {
                return Mat3::Identity() * json.number_value();
            }
            else if (json.is_array() && json.array_items().size() == 9)
            {
                return interpret_json_as_mat3(json);
            }
            else
            {
                std::cerr << "LayerInfo parse error" << std::endl;
                exit(1);
            }
        }
        
        Json interpret_comp_op_as_json(const CompOp& comp_op)
        {
            if (comp_op.is_plus()) { return Json("plus"); }
            if (comp_op.is_source_over()) { return Json("source-over"); }
            
            return Json("unknown");
        }
        
        Json interpret_color_model_as_json(const ColorModelPtr color_model)
        {
            const GaussianColorModel* gaussian = dynamic_cast<GaussianColorModel*>(color_model.get());
            assert(gaussian != nullptr);
            
            return Json::object
            {
                { "primary_color",  interpret_vecx_as_json(gaussian->get_mu()) },
                { "color_variance", interpret_variance_as_json(gaussian->get_sigma()) }
            };
        }
        
        Json interpret_blend_mode_as_json(const BlendMode mode)
        {
            return Json(retrieve_name(mode));
        }
        
        Json interpret_layer_info_as_json(const LayerInfo& layer_info)
        {
            return Json::object
            {
                { "mode",        interpret_blend_mode_as_json(layer_info.blend_mode) },
                { "color_model", interpret_color_model_as_json(layer_info.color_model) },
                { "comp_op",     interpret_comp_op_as_json(layer_info.comp_op) }
            };
        }
        
        vector<Json> interpret_layer_infos_as_json(const vector<LayerInfo>& layer_infos)
        {
            vector<Json> json_vec;
            for (int i = 0; i < layer_infos.size(); ++ i)
            {
                const auto& info = layer_infos[i];
                json_vec.push_back(interpret_layer_info_as_json(info));
            }
            return json_vec;
        }
    }
    
    void export_layers(const std::vector<ColorImage>& layers,
                       const std::string&             output_directory_path,
                       const std::string&             file_name_prefix,
                       const bool                     with_alpha_channel,
                       const bool                     with_blend_mode_suffix,
                       const std::vector<LayerInfo>&  layer_infos)
    {
        assert((!with_blend_mode_suffix) || layer_infos.size() == layers.size());
        
        for (int index = 0; index < layers.size(); ++ index)
        {
            const std::string suffix = with_blend_mode_suffix ? "_" + retrieve_name(layer_infos[index].blend_mode) : "";
            layers[index].save(output_directory_path + "/" + file_name_prefix + "_" + std::to_string(index) + suffix + ".png");
            if (with_alpha_channel)
            {
                layers[index].get_a().save(output_directory_path + "/" + file_name_prefix + "-alpha_" + std::to_string(index) + ".png");
            }
        }
    }
    
    void export_models(const vector<ColorModelPtr>& models,
                       const std::string&           output_directory_path,
                       const std::string&           file_name_prefix)
    {
        for (int index = 0; index < models.size(); ++ index)
        {
            models[index]->generate_visualization().save(output_directory_path + "/" + file_name_prefix + "_" + std::to_string(index) + ".png");
        }
    }
    
    void export_layer_infos(const std::vector<LayerInfo>& layer_infos,
                            const std::string& output_directory_path)
    {
        const Json json_object = interpret_layer_infos_as_json(layer_infos);
        
        std::ofstream writing_file(output_directory_path + "/layer_infos.json");
        writing_file << json_object.dump();
    }
    
    std::vector<LayerInfo> import_layer_infos(const std::string& input_file_path)
    {
        std::ifstream reading_file(input_file_path);
        const std::string json_text = std::string(std::istreambuf_iterator<char>(reading_file), std::istreambuf_iterator<char>());;
        
        std::string err;
        const auto json = Json::parse(json_text, err);
        
        std::vector<LayerInfo> layer_infos;
        
        for (const auto& layer_info_json : json.array_items())
        {
            const std::string mode_name    = layer_info_json["mode"].string_value();
            const std::string comp_op_name = layer_info_json["comp_op"].string_value();
            
            const CompOp    comp_op = (comp_op_name == "source-over") ? CompOp::SourceOver() : CompOp::Plus();
            const BlendMode mode    = retrieve_by_name(mode_name);
            
            const auto& model = layer_info_json["color_model"];
            
            const Vec3 mu    = interpret_json_as_vec3    (model["primary_color" ]);
            const Mat3 sigma = interpret_json_as_variance(model["color_variance"]);
            
            layer_infos.push_back(LayerInfo{ comp_op, mode, std::make_shared<GaussianColorModel>(mu, sigma.inverse()) });
        }
        
        return layer_infos;
    }
}
