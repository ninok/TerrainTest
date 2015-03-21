#ifndef RENDERINGWIDGET_H
#define RENDERINGWIDGET_H

#include <QBasicTimer>

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

class QOpenGLTexture;
class QOpenGLShader;
class QOpenGLShaderProgram;

struct Vertex
{
    GLfloat x;
    GLfloat y;
    GLfloat z;

    GLfloat u;
    GLfloat v;

    GLfloat n_x;
    GLfloat n_y;
    GLfloat n_z;
};


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

    void timerEvent( QTimerEvent* e ) override;

    void keyPressEvent( QKeyEvent* event ) override;

    void mouseMoveEvent( QMouseEvent* event ) override;

    void wheelEvent( QWheelEvent* event ) override;

signals:

public slots:

private:

    QBasicTimer m_timer;

    QOpenGLBuffer m_terrain_vbo;
    QOpenGLBuffer m_terrain_ibo;

    QImage m_heightmap;
    QScopedPointer< QOpenGLTexture > m_texture;

    QScopedPointer< QOpenGLShader > m_terrain_vertex_shader;
    QScopedPointer< QOpenGLShader > m_terrain_fragment_shader;
    QScopedPointer< QOpenGLShaderProgram > m_terrain_shader_program;

    glm::mat4x4 m_projection_matrix;

    std::vector< GLuint > m_indices;
    std::vector< Vertex > m_vertices;

    float m_scale;
    glm::vec3 m_center;
    glm::vec3 m_eye;
    glm::vec3 m_up;

    size_t m_max_index;
    std::pair< size_t, size_t > m_index_range;
};

#endif // RENDERINGWIDGET_H
