#ifndef RENDERINGWIDGET_H
#define RENDERINGWIDGET_H

#include <QBasicTimer>

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QMatrix4x4>

class QOpenGLTexture;
class QOpenGLShader;
class QOpenGLShaderProgram;

class RenderingWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit RenderingWidget(QWidget *parent = 0);
    ~RenderingWidget();

protected:
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void initializeGL() override;

    void init_terrain( );
    void init_terrain_shaders( );
    void init_terrain_vbo( );

    void timerEvent( QTimerEvent *e ) override;

signals:

public slots:

private:

    QBasicTimer m_timer;

    QOpenGLBuffer m_terrain_vbo;
    QScopedPointer< QOpenGLTexture > m_texture;

    QScopedPointer< QOpenGLShader > m_terrain_vertex_shader;
    QScopedPointer< QOpenGLShader > m_terrain_fragment_shader;
    QScopedPointer< QOpenGLShaderProgram > m_terrain_shader_program;

    QMatrix4x4 m_projection;

	std::vector< float > m_vertices;
	std::vector< float > m_tex_coords;
	std::vector< float > m_normals;


    float m_scale;
    float m_angle;
};

#endif // RENDERINGWIDGET_H
