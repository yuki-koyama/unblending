#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class LayerWidget;
class ImageWidget;

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private slots:
    void on_actionDecompose_triggered();
    
    void on_actionCreateLayer_triggered();
    
    void on_actionDeleteLayer_triggered();
    
    void on_actionImport_triggered();
    
    void on_actionExport_triggered();
    
    void on_actionCapture_triggered();
    
private:
    Ui::MainWindow *ui;
    
    void build_layer_widgets();
    
    std::vector<LayerWidget*> layer_widgets_;
    ImageWidget* main_image_widget_;
};

#endif // MAINWINDOW_H
