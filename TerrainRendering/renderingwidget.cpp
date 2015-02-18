#include "renderingwidget.h"

#include <QPainter>

#include <QOpenGLTexture>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>

RenderingWidget::RenderingWidget(QWidget *parent)
    : QOpenGLWidget(parent),
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
    modelview_matrix.translate(0.0, 0.0, -5.0);
    //matrix.rotate(rotation);

    modelview_matrix.rotate(m_angle, 0.0f, 1.0f, 0.0f);
    modelview_matrix.rotate(m_angle, 1.0f, 0.0f, 0.0f);
    modelview_matrix.rotate(m_angle, 0.0f, 0.0f, 1.0f);
    modelview_matrix.scale(m_scale);
    modelview_matrix.translate(0.0f, -0.2f, 0.0f);

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
    m_terrain_shader_program->setAttributeBuffer(vertex_attr, GL_FLOAT, 0, 3);
    m_terrain_shader_program->setAttributeBuffer(tex_coord_attr, GL_FLOAT, 36 * 3 * sizeof(GLfloat), 2);
    m_terrain_shader_program->setAttributeBuffer(normal_attr, GL_FLOAT, 36 * 5 * sizeof(GLfloat), 3);
    m_terrain_vbo.release();


    glEnable(GL_DEPTH_TEST);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, m_vertices.size( ) / 3 );
    glEnable(GL_DEPTH_TEST);

    m_terrain_shader_program->release( );

    // Draw cube geometry
    //geometries->drawCubeGeometry(&program);

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
    const char *vsrc =
        "attribute highp vec4 vertex;\n"
        "attribute highp vec4 texCoord;\n"
        "attribute mediump vec3 normal;\n"
        "uniform mediump mat4 MV;\n"
        "uniform mediump mat4 MVP;\n"
        "varying highp vec4 texc;\n"
        "varying mediump float angle;\n"
        "void main(void)\n"
        "{\n"
        "    vec3 toLight = normalize(vec3(0.0, 0.3, 1.0));\n"
		"    angle = max(dot( normal, (MV*vec4(toLight,0.0))), 0.0);\n"
        "    gl_Position = MVP * vertex;\n"
        "    texc = texCoord;\n"
        "}\n";
	Q_ASSERT(m_terrain_vertex_shader->compileSourceCode(vsrc));

    m_terrain_fragment_shader.reset( new QOpenGLShader(QOpenGLShader::Fragment) );
    const char *fsrc =
        "varying highp vec4 texc;\n"
        "uniform sampler2D texture;\n"
        "varying mediump float angle;\n"
        "void main(void)\n"
        "{\n"
        "    highp vec3 color = texture2D(texture, texc.st).rgb;\n"
        "    color = color * 0.2 + color * 0.8 * angle;\n"
        "    gl_FragColor = vec4(clamp(color, 0.0, 1.0), 1.0);\n"
        "}\n";
	Q_ASSERT(m_terrain_fragment_shader->compileSourceCode(fsrc));

    m_terrain_shader_program.reset( new QOpenGLShaderProgram );
    m_terrain_shader_program->addShader(m_terrain_vertex_shader.data( ) );
    m_terrain_shader_program->addShader(m_terrain_fragment_shader.data( ) );
	Q_ASSERT(m_terrain_shader_program->link());
}

void RenderingWidget::init_terrain_vbo()
{
    Q_ASSERT( !m_terrain_vbo.isCreated( ) );

	for (size_t i = 0; i < 100; ++i)
	{
		const float x = i / 100.0f;
		const float y = i % 2 ? 0.0f : 1.0f;
		const float z = 0;

		m_vertices.push_back(x);
		m_vertices.push_back(y);
		m_vertices.push_back(z);

		m_tex_coords.push_back(x);
		m_tex_coords.push_back(y);

		m_normals.push_back(0.0);
		m_normals.push_back(0.0);
		m_normals.push_back(1.0);
	}

    m_terrain_vbo.create();
    m_terrain_vbo.bind();
    m_terrain_vbo.allocate((int)m_vertices.size( ) * 8 * sizeof(GLfloat));

	const int vertices_size = sizeof(GLfloat) * m_vertices.size();
	const int texcoord_size = sizeof(GLfloat) * m_tex_coords.size();
	const int normal_size = sizeof(GLfloat) * m_normals.size();
	m_terrain_vbo.write(0, m_vertices.data(), vertices_size );
	m_terrain_vbo.write(vertices_size, m_tex_coords.data( ), texcoord_size );
	m_terrain_vbo.write(vertices_size + texcoord_size, m_normals.data() , normal_size );
    m_terrain_vbo.release();
}

void RenderingWidget::timerEvent(QTimerEvent *e)
{
    m_angle+=1.0f;
    update( );
}

