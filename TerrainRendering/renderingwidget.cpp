#include "renderingwidget.h"

#include <QPainter>
#include <QKeyEvent>

#include <QOpenGLTexture>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>

#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr

#include <glm/mat3x3.hpp>

#include <stdint.h>

uint32_t
interleave_bits(uint16_t x, uint16_t y)
{
    x = ((x << 8) | x) & 0x00FF00FFU;
    x = ((x << 4) | x) & 0x0F0F0F0FU;
    x = ((x << 2) | x) & 0x33333333U;
    x = ((x << 1) | x) & 0x55555555U;

    y = ((y << 8) | y) & 0x00FF00FFU;
    y = ((y << 4) | y) & 0x0F0F0F0FU;
    y = ((y << 2) | y) & 0x33333333U;
    y = ((y << 1) | y) & 0x55555555U;

    return (x | (y << 1));
}

// -------------------------------------------------------------------------------------------------

void
deinterleave_bits(uint16_t val, uint16_t& x, uint16_t& y)
{
    x = val & 0x55555555U;
    x = ((x >> 1) | x) & 0x33333333U;
    x = ((x >> 2) | x) & 0x0F0F0F0FU;
    x = ((x >> 4) | x) & 0x00FF00FFU;
    x = ((x >> 8) | x) & 0x0000FFFFU;

    y = (val >> 1) & 0x55555555U;
    y = ((y >> 1) | y) & 0x33333333U;
    y = ((y >> 2) | y) & 0x0F0F0F0FU;
    y = ((y >> 4) | y) & 0x00FF00FFU;
    y = ((y >> 8) | y) & 0x0000FFFFU;
}

// -------------------------------------------------------------------------------------------------

RenderingWidget::RenderingWidget(QWidget *parent)
    : QOpenGLWidget(parent),
      m_terrain_vbo( QOpenGLBuffer::VertexBuffer ),
      m_terrain_ibo( QOpenGLBuffer::IndexBuffer ),
      m_eye(0.0, 0.0, -5.0),
      m_center(0.0, 0.0, 0.0),
      m_up(0.0, 1.0, 0.0),
      m_scale( 1.0f ),
      m_max_index( 0 ),
      m_index_range( 0, 0)
{
    setFocusPolicy( Qt::StrongFocus );
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

    m_projection_matrix = glm::perspective( fov, aspect, zNear, zFar);
}

void RenderingWidget::paintGL()
{
    QPainter painter(this);
    painter.beginNativePainting();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Calculate model view transformation
    glm::mat4 view = glm::lookAt(m_eye, m_center, m_up);
    glm::mat4 model(1.0f);
    //model = glm::rotate(model, glm::radians(m_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    //model = glm::rotate(model, glm::radians(m_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    //model = glm::rotate(model, glm::radians(m_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(m_scale, m_scale, m_scale));
    //model = glm::translate(model, glm::vec3(0.0, 0.0, 2.0));

    m_texture->bind( );
    m_terrain_shader_program->bind( );
    m_terrain_shader_program->setUniformValue("texture", 0);

    glUniformMatrix4fv(m_terrain_shader_program->uniformLocation("MV"), 1, GL_FALSE, glm::value_ptr(view * model));
    glUniformMatrix4fv(m_terrain_shader_program->uniformLocation("MVP"), 1, GL_FALSE, glm::value_ptr(m_projection_matrix * view * model));
    //m_terrain_shader_program->setUniformValue("MV", QMatrix4x4(glm::value_ptr(modelview_matrix)));
    //m_terrain_shader_program->setUniformValue("MVP", QMatrix4x4(glm::value_ptr(m_projection_matrix * modelview_matrix)));

    int vertex_attr = m_terrain_shader_program->attributeLocation("vertex");
    int normal_attr = m_terrain_shader_program->attributeLocation("normal");
    int tex_coord_attr = m_terrain_shader_program->attributeLocation("tex_coords");

    m_terrain_shader_program->enableAttributeArray(vertex_attr);
    m_terrain_shader_program->enableAttributeArray(normal_attr);
    m_terrain_shader_program->enableAttributeArray(tex_coord_attr);

    m_terrain_vbo.bind();
    m_terrain_ibo.bind();

    m_terrain_shader_program->setAttributeBuffer(vertex_attr, GL_FLOAT, 0, 3, sizeof(Vertex));
    m_terrain_shader_program->setAttributeBuffer(tex_coord_attr, GL_FLOAT, 3 * sizeof(GLfloat), 2, sizeof(Vertex));
    m_terrain_shader_program->setAttributeBuffer(normal_attr, GL_FLOAT, 5 * sizeof(GLfloat), 3, sizeof(Vertex));

    glEnable(GL_DEPTH_TEST);
    const size_t quad_count = ( m_heightmap.width( ) - 1 ) *( m_heightmap.height( ) - 1 );
    glDrawElements(GL_TRIANGLES, (int)(quad_count * 6) , GL_UNSIGNED_INT, (void*)0 );

    glEnable(GL_DEPTH_TEST);

    m_terrain_ibo.release();
    m_terrain_vbo.release();

    m_terrain_shader_program->release( );

    painter.endNativePainting();
}

void RenderingWidget::initializeGL()
{
    initializeOpenGLFunctions();

    m_heightmap.load( ":/heightmap.png" );
    m_texture.reset( new QOpenGLTexture( m_heightmap ) );

    init_terrain( );

    glClearColor(0, 0, 0, 1);

    m_timer.start(10, this);
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

    const size_t width = m_heightmap.width( );
    const size_t height = m_heightmap.height( );

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

    Q_ASSERT(width <= (size_t)std::numeric_limits<uint16_t>::max);
    Q_ASSERT(height <= (size_t)std::numeric_limits<uint16_t>::max);

    m_max_index = width * height * 6;
    m_indices.resize( m_max_index );
    for (size_t iy = 0; iy < height - 1; ++iy)
    {
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

            //size_t index = interleave_bits(ix, iy) * 6;
            size_t index = (iy * (width - 1) + ix) * 6;

            m_indices[index++] = idx_0;
            m_indices[index++] = idx_1;
            m_indices[index++] = idx_2;

            m_indices[index++] = idx_0;
            m_indices[index++] = idx_2;
            m_indices[index++] = idx_3;
        }
    }

    const int indices_size = (int)(m_indices.size( ) * sizeof(GLuint) );
    m_terrain_ibo.create();
    m_terrain_ibo.bind();
    m_terrain_ibo.allocate( indices_size );
    m_terrain_ibo.write(0, m_indices.data(), indices_size );
    m_terrain_ibo.release( );
}

void RenderingWidget::timerEvent(QTimerEvent*)
{
    m_index_range.second += 6;
    if (m_index_range.second > m_max_index)
        m_index_range.second = 1;

    //m_angle+=1.0f;
    update( );
}

void RenderingWidget::keyPressEvent(QKeyEvent* e)
{
    using namespace glm;
    const vec3 ahead = normalize( m_center - m_eye );
    const vec3 left = normalize( cross(ahead, m_up) );

    switch (e->key())
    {
    case Qt::Key_Plus:
        m_scale += 0.1f;
        update();
        break;
    case Qt::Key_Minus:
        m_scale = std::max( m_scale - 0.1f, 0.1f );
        update();
        break;
    case Qt::Key_W:
    case Qt::Key_Up:
        m_eye += 0.1f * ahead;
        m_center += 0.1f * ahead;
        update();
        break;
    case Qt::Key_S:
    case Qt::Key_Down:
        m_eye -= 0.1f * ahead;
        m_center -= 0.1f * ahead;
        update();
        break;
    case Qt::Key_A:
        m_eye += 0.1f * left;
        m_center += 0.1f * left;
        update();
        break;
    case Qt::Key_D:
        m_eye -= 0.1f * left;
        m_center -= 0.1f * left;
        update();
        break;
    case Qt::Key_Left:
        m_center = m_eye + rotate(ahead, radians(1.0f), m_up);
        update();
        break;
    case Qt::Key_Right:
        m_center = m_eye + rotate(ahead, radians(-1.0f), m_up);
        update();
        break;
    }
}

void RenderingWidget::mouseMoveEvent( QMouseEvent* event )
{
    event->accept( );
}


void RenderingWidget::wheelEvent( QWheelEvent* event )
{
    event->accept( );
}
