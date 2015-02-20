#include "renderingwidget.h"

#include <QPainter>

#include <QOpenGLTexture>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>

RenderingWidget::RenderingWidget(QWidget *parent)
    : QOpenGLWidget(parent),
      m_terrain_vbo( QOpenGLBuffer::VertexBuffer ),
      m_terrain_ibo( QOpenGLBuffer::IndexBuffer ),
      m_angle( 0.0f ),
      m_scale( 1.0f )
{

}

RenderingWidget::~RenderingWidget()
{
    if ( !m_texture.isNull( ) )
    {
        makeCurrent();
        m_texture->destroy( );
    }
}

void RenderingWidget::resizeGL(int w, int h)
{
    // Calculate aspect ratio
    qreal aspect = qreal(w) / qreal(h ? h : 1);

    // Set near plane to 3.0, far plane to 7.0, field of view 45 degrees
    const qreal zNear = 3.0, zFar = 7.0, fov = 45.0;

    // Reset projection
    m_projection.setToIdentity();

    // Set perspective projection
    m_projection.perspective(fov, aspect, zNear, zFar);
}

void RenderingWidget::paintGL()
{
    QPainter painter(this);
    painter.beginNativePainting();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Calculate model view transformation
    QMatrix4x4 modelview_matrix;

    modelview_matrix.translate(0.0, 0.0, -4.0);
    modelview_matrix.rotate(m_angle, 0.0f, 1.0f, 0.0f);
    modelview_matrix.rotate( -65.0f, 1.0f, 0.0f, 0.0f);
    //modelview_matrix.rotate(m_angle, 0.0f, 0.0f, 1.0f);
    modelview_matrix.scale(m_scale);
    //modelview_matrix.translate(0.0f, -0.2f, 0.0f);

    m_texture->bind( );
    m_terrain_shader_program->bind( );
    m_terrain_shader_program->setUniformValue("texture", 0);
    m_terrain_shader_program->setUniformValue("MV", modelview_matrix);
    m_terrain_shader_program->setUniformValue("MVP", m_projection * modelview_matrix);
    //m_matrixUniform2 = m_terrain_shader_program->uniformLocation("matrix");
    //m_textureUniform2 = m_terrain_shader_program->uniformLocation("texture");

    int vertex_attr = m_terrain_shader_program->attributeLocation("vertex");
    int normal_attr = m_terrain_shader_program->attributeLocation("normal");
    int tex_coord_attr = m_terrain_shader_program->attributeLocation("texCoord");

    m_terrain_shader_program->enableAttributeArray(vertex_attr);
    m_terrain_shader_program->enableAttributeArray(normal_attr);
    m_terrain_shader_program->enableAttributeArray(tex_coord_attr);

    m_terrain_vbo.bind();
    m_terrain_ibo.bind();

    m_terrain_shader_program->setAttributeBuffer(vertex_attr, GL_FLOAT, 0, 3, sizeof(Vertex));
    m_terrain_shader_program->setAttributeBuffer(tex_coord_attr, GL_FLOAT, 3 * sizeof(GLfloat), 2, sizeof(Vertex));
    m_terrain_shader_program->setAttributeBuffer(normal_attr, GL_FLOAT, 5 * sizeof(GLfloat), 3, sizeof(Vertex));

    glEnable(GL_DEPTH_TEST);
    glDrawElements(GL_TRIANGLES, (int)m_indices.size( ), GL_UNSIGNED_INT, nullptr );
    glEnable(GL_DEPTH_TEST);

    m_terrain_ibo.release();
    m_terrain_vbo.release();

    m_terrain_shader_program->release( );

    painter.endNativePainting();
}

void RenderingWidget::initializeGL()
{
    initializeOpenGLFunctions();

    init_terrain( );

    m_texture.reset( new QOpenGLTexture( QImage(":/qt.png") ) );

    glClearColor(0, 0, 0, 1);

    m_timer.start(12, this);
}

void RenderingWidget::init_terrain()
{
    init_terrain_shaders( );
    init_terrain_vbo( );
}

void RenderingWidget::init_terrain_shaders()
{

    m_terrain_vertex_shader.reset( new QOpenGLShader(QOpenGLShader::Vertex) );
    Q_ASSERT(m_terrain_vertex_shader->compileSourceFile( "vs_terrain.glsl" ) );

    m_terrain_fragment_shader.reset( new QOpenGLShader(QOpenGLShader::Fragment) );
    Q_ASSERT(m_terrain_fragment_shader->compileSourceFile( "fs_terrain.glsl"));

    m_terrain_shader_program.reset( new QOpenGLShaderProgram );
    m_terrain_shader_program->addShader(m_terrain_vertex_shader.data( ) );
    m_terrain_shader_program->addShader(m_terrain_fragment_shader.data( ) );
    Q_ASSERT(m_terrain_shader_program->link());
}


void RenderingWidget::init_terrain_vbo()
{
    Q_ASSERT( !m_terrain_vbo.isCreated( ) );

    const size_t width = 129;
    const size_t height = 129;

    for (size_t iy = 0; iy < height; ++iy)
    {
        const float y = iy / (height - 1.0f);

        for (size_t ix = 0; ix < width; ++ix)
        {
            const float x = ix / (width - 1.0f);

            const float z = 0.0f;

            const Vertex vertex = {
                x - 0.5f, y - 0.5f, z,
                x, y,
                0.0f, 0.0f, 1.0f
            };

            m_vertices.push_back(vertex);
        }
    }

    m_terrain_vbo.create();
    m_terrain_vbo.bind();
    m_terrain_vbo.allocate( m_vertices.size() * sizeof(Vertex) );

    m_terrain_vbo.write(0, m_vertices.data(), m_vertices.size( ) * sizeof( Vertex ) );
    m_terrain_vbo.release();

    for (size_t iy = 0; iy < height - 1; ++iy)
    {
        const float y = iy / (height - 1.0f);

        for (size_t ix = 0; ix < width - 1; ++ix)
        {
            // 0---3
            // |   |
            // |   |
            // |   |
            // 1---2

            const GLuint idx_0 = iy * width + ix;
            const GLuint idx_1 = (iy+1) * width + ix;
            const GLuint idx_2 = (iy+1) * width + ix + 1;
            const GLuint idx_3 = iy * width + ix + 1;

            m_indices.push_back( idx_0 );
            m_indices.push_back( idx_1 );
            m_indices.push_back( idx_2 );

            m_indices.push_back( idx_0 );
            m_indices.push_back( idx_2 );
            m_indices.push_back( idx_3 );
        }
    }

    const int indices_size = (int)(m_indices.size( ) * sizeof(GLuint) );

    m_terrain_ibo.create();
    m_terrain_ibo.bind();
    m_terrain_ibo.allocate( indices_size );
    m_terrain_ibo.write(0, m_indices.data(), indices_size );
    m_terrain_ibo.release( );

}

void RenderingWidget::timerEvent(QTimerEvent *e)
{
    m_angle+=1.0f;
    update( );
}

