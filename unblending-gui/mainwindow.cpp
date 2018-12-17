#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <QFileDialog>
#include <QLabel>
#include <QPushButton>
#include <QProgressDialog>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <unblending/common.hpp>
#include "core.h"
#include "imagewidget.h"
#include "layerwidget.h"
#include "layerimagewidget.h"
#include "layercolormodelwidget.h"

namespace
{
    Core& core = Core::GetInstance();
}

MainWindow::MainWindow(QWidget *parent) :
QMainWindow(parent),
ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    build_layer_widgets();
    
    main_image_widget_ = new ImageWidget(this);
    main_image_widget_->copy_and_set_image(*core.get_image());
    
    QPushButton* button = new QPushButton(QString("Run Decomposition"), this);
    connect(button, SIGNAL(released()), this, SLOT(on_actionDecompose_triggered()));
    
    ui->mainVerticalLayout->addWidget(new QLabel(QString("Target image:"), this));
    ui->mainVerticalLayout->addWidget(main_image_widget_);
    ui->mainVerticalLayout->addWidget([]()
                                      {
                                          QFrame* line = new QFrame();
                                          line->setFrameShape(QFrame::HLine);
                                          line->setFrameShadow(QFrame::Sunken);
                                          return line;
                                      }());
    ui->mainVerticalLayout->addWidget(button);
    ui->mainVerticalLayout->addStretch();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::build_layer_widgets()
{
    // Clear all the items if exist
    std::function<void(QLayout*)> clear_layout = [&](QLayout* layout) -> void
    {
        while (QLayoutItem* item = layout->takeAt(0))
        {
            if (QLayout* child_layout = item->layout())
            {
                clear_layout(child_layout);
            }
            if (item->widget()) delete item->widget();
            delete item;
        }
    };
    clear_layout(ui->scrollAreaHorizontalLayout);
    
    auto& modes = core.get_modes();
    
    const auto& models         = core.get_models();
    const auto  visualizations = core.generate_color_model_visualizations();
    const int   num_layers     = visualizations.size();
    
    layer_widgets_.resize(num_layers);
    
    for (int i = 0; i < num_layers; ++ i)
    {
        LayerWidget* layer_widget = new LayerWidget(i, &(modes[i]), models[i], this);
        layer_widgets_[i] = layer_widget;
        
        LayerImageWidget* layer_image_widget = new LayerImageWidget(layer_widget);
        layer_widget->set_layer_image_widget(layer_image_widget);
        
        layer_widget->initialize_blend_model_option();
        
        LayerColorModelWidget* layer_color_model_widget = new LayerColorModelWidget(layer_widget);
        layer_color_model_widget->copy_and_set_image(visualizations[i]);
        layer_color_model_widget->setFixedSize(120, 120);
        layer_widget->set_layer_color_model_widget(layer_color_model_widget);
        
        layer_widget->initialize_control_widget();
        layer_widget->insert_stretch();
        
        ui->scrollAreaHorizontalLayout->addWidget(layer_widget);
    }
    
    ui->scrollAreaHorizontalLayout->addStretch(1);
}

void MainWindow::on_actionDecompose_triggered()
{
    auto background_process = [&]()
    {
        core.decompose_image();
    };
    
    {
        QProgressDialog dialog(QString("Decomposing..."), QString(), 0, 0, this);
        QFutureWatcher<void> watcher;
        QObject::connect(&watcher, SIGNAL(finished()), &dialog, SLOT(reset()));
        watcher.setFuture(QtConcurrent::run(background_process));
        dialog.exec();
        watcher.waitForFinished();
    }
    
    const auto layers     = core.get_layers();
    const int  num_layers = layers.size();
    
    for (int i = 0; i < num_layers; ++ i)
    {
        layer_widgets_[i]->layer_image_widget_->copy_and_set_image(layers[i]);
    }
    
    repaint();
}

void MainWindow::on_actionCreateLayer_triggered()
{
    core.create_color_model();
    build_layer_widgets();
}

void MainWindow::on_actionDeleteLayer_triggered()
{
    core.delete_color_model();
    build_layer_widgets();
}

void MainWindow::on_actionImport_triggered()
{
    const std::string image_file_path = QFileDialog::getOpenFileName(this).toStdString();
    core.import_image(image_file_path);
    main_image_widget_->copy_and_set_image(*core.get_image());
}

void MainWindow::on_actionExport_triggered()
{
    const std::string output_dir_path = QFileDialog::getExistingDirectory(this).toStdString();
    core.export_files(output_dir_path);
}

void MainWindow::on_actionCapture_triggered()
{
    const std::string output_dir_path    = QFileDialog::getExistingDirectory().toStdString();
    const std::string capture_image_name = unblending::get_current_time_in_string() + ".png";
    this->grab().save(QString::fromStdString(output_dir_path) + "/" + QString::fromStdString(capture_image_name));
}
