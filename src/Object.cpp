#include "Object.h"

Object::Object() :
	m_mesh(nullptr), m_shader(nullptr), m_soft(nullptr),
	m_click(false), m_delete(false), m_name(""), m_property({}), m_path(""), m_id(0)
{
	m_property.push_back(glm::vec3(0.0f)); // Position
	m_property.push_back(glm::vec3(0.0f)); // Rotation
	m_property.push_back(glm::vec3(1.0f)); // Scale
}

Object::Object(const string& mesh_path) :
	m_soft(nullptr),
	m_click(false), m_delete(false), m_name(""), m_property({}), m_path(mesh_path), m_id(0)
{
	int last = int(mesh_path.find_last_of('/'));
	if (last == -1)
	{
		last = int(mesh_path.find_last_of('\\'));
	}
	string temp = mesh_path.substr(last + 1, mesh_path.length());
	m_name = temp.substr(0, temp.find_last_of('.'));

	string::size_type pos = mesh_path.find_last_of('.');
	if (pos != mesh_path.length())
	{
		string file_type = mesh_path.substr(pos + 1, mesh_path.length());
		cout << "GameObject type : " << file_type << endl;
		if (file_type == "txt")
		{
			m_mesh = make_shared<Mesh>(m_name, mesh_path);
		}
		else if (file_type == "fbx")
		{
			m_mesh = make_shared<FBXMesh>(mesh_path);
		}
	}
	else
	{
		cerr << "Invalid file " << mesh_path << endl;
		assert(0);
	}
	m_mesh->processMesh();

	m_property.push_back(glm::vec3(0.0f)); // Position
	m_property.push_back(glm::vec3(0.0f)); // Rotation
	m_property.push_back(glm::vec3(1.0f)); // Scale

	vector<string> shader_path = { "assets/shaders/BRDF.vert", "assets/shaders/BRDF.frag" };
	m_shader = make_shared<Shader>(shader_path);
	m_shader->processShader();
}

Object::Object(const string& mesh_path, const vector<string>& shader) :
	m_soft(nullptr), 
	m_click(false), m_delete(false), m_name(""), m_property({}), m_path(mesh_path), m_id(0)
{
	int last = int(mesh_path.find_last_of('/'));
	if (last == -1)
	{
		last = int(mesh_path.find_last_of('\\'));
	}
	string temp = mesh_path.substr(last + 1, mesh_path.length());
	m_name = temp.substr(0, temp.find_last_of('.'));

	string::size_type pos = mesh_path.find_last_of('.');
	if (pos != mesh_path.length())
	{
		string file_type = mesh_path.substr(pos + 1, mesh_path.length());
		cout << "GameObject type : " << file_type << endl;
		if (file_type == "txt")
		{
			m_mesh = make_shared<Mesh>(m_name, mesh_path);
		}
		else if (file_type == "fbx")
		{
			m_mesh = make_shared<FBXMesh>(mesh_path);
		}
	}
	else
	{
		cerr << "Invalid file " << mesh_path << endl;
		assert(0);
	}
	m_mesh->processMesh();

	m_property.push_back(glm::vec3(0.0f)); // Position
	m_property.push_back(glm::vec3(0.0f)); // Rotation
	m_property.push_back(glm::vec3(1.0f)); // Scale

	m_shader = make_shared<Shader>(shader);
	m_shader->processShader();
}

Object::~Object()
{}

bool Object::isClick(glm::vec3& ray_dir, glm::vec3& ray_pos)
{
	if (m_mesh == nullptr) return false;

	m_click = m_mesh->intersect(ray_dir, ray_pos);
	return m_click;
}

void Object::setProperty(int index, glm::vec3 t)
{
	switch (index)
	{
	case(0):
		// Set Position
		setPosition(t);
		break;

	case(1):
		// Set Rotation
		setRotation(t);
		break;

	case(2):
		setScale(t);
		break;

	default:
		cerr << "Wrong property" << endl;
		assert(0);
		break;
	}
}

void Object::setPosition(glm::vec3 pos)
{
	m_property[0] = pos;
	glm::mat4 t = glm::mat4(1.0f);
	t = glm::translate(t, m_property[0]);
	m_mesh->setPosition(t);
};

void Object::setRotation(glm::vec3 rot)
{
	//cout << m_name << " : set rotation " << rot <<  endl;
	m_property[1] = rot;
	glm::mat4 t = glm::mat4(1.0f);
	
	for (int i = 0; i < 3; ++i)
	{
		glm::vec3 axis = glm::vec3(0.0f);
		axis[i] = 1.0f;
		//cout << "Axis-" << i << " : " << endl;
		float angle = glm::radians(m_property[1][i]);
		//t = glm::rotate(t, angle, axis);
		Quaternion q;
		q.set(axis, angle);
		glm::mat4 m = q.getMatrix();
		t *= m;
		//cout << m << endl;
	}
	m_mesh->setRotation(t);
};

void Object::setScale(glm::vec3 scale)
{
	m_property[2] = glm::abs(scale);
	glm::mat4 t = glm::mat4(1.0f);
	t = glm::scale(t, m_property[2]);
	m_mesh->setScale(t);
};

Gizmo::Gizmo(int axis) : m_axis(axis)
{
	cout << "Gizmo constructor " << axis << endl;

	if (axis == 3)
	{
		m_mesh = make_shared<Mesh>("Gizmo_Center", "assets/models/Cube.txt");
	}
	else
	{
		m_mesh = make_shared<FBXMesh>("assets/models/Arrow.fbx");
	}
	m_mesh->processMesh();

	vector<string> shader_paths = {"assets/shaders/Arrow.vert", "assets/shaders/Arrow.frag"};
	m_shader = make_shared<Shader>(shader_paths);
	m_shader->processShader();
}

Gizmo::~Gizmo()
{}

void Gizmo::draw(GameObject& go, const glm::mat4& P, const glm::mat4& V, glm::vec3 cam_pos)
{
	//cout << "Draw: " << m_axis << endl;

	glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f);

	glm::mat4 M = glm::mat4(1.0f);
	glm::vec3 scale = glm::vec3(glm::length(m_property[0] - cam_pos) * 0.05f);
	
	if (m_axis == 3)
		scale *= 0.2f;

	M = glm::scale(M, scale);
	M = *(go.getMesh()->getPosition()) * M;
	
	if (m_axis == 1)
		M = glm::rotate(M, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	else if (m_axis == 2)
		M = glm::rotate(M, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	
	if (m_axis != 3)
	{
		if (m_click)
			color[m_axis] = 0.3f;
		else
			color[m_axis] = 0.7f;	
	}
	else
	{
		color = glm::vec3(1.0f);
	}
	
	m_mesh->setPosition(M);

	m_shader->load();
	m_shader->setVec3("object_color", color);
	m_mesh->draw(P, V, *m_shader);
}

bool Gizmo::isClick(glm::vec3& ray_dir, glm::vec3& ray_pos)
{
	if (m_axis == 3) return false;

	return m_mesh->intersect(ray_dir, ray_pos);
}

Grid::Grid()
{
	m_name = "Grid";
	m_mesh = make_shared<Mesh>(m_name, "assets/models/Grid.txt");
	m_mesh->processMesh();
	
	vector<string> shader_path = { "assets/shaders/Grid.vert", "assets/shaders/Grid.frag" };
	m_shader = make_shared<Shader>(shader_path);
	m_shader->processShader();

	cout << "Grid finish loading..." << endl;
	cout << endl;
}

void Grid::draw(const glm::mat4& P, const glm::mat4& V, glm::vec3 cam_pos)
{
	m_shader->load();
	glm::mat4 M = glm::mat4(1.0f);
	m_shader->setPVM(P, V, M);
	m_shader->setVec3("cam_pos", cam_pos);
	m_mesh->draw();
}

GameObject::GameObject() : 
	Object(), m_is_popup(false),
	m_color(glm::vec3(1.0f, 0.5f, 0.31f)), 
	m_move_axis(-1)
{}

GameObject::GameObject(const string& mesh_path) : 
	Object(mesh_path), m_is_popup(false),
	m_color(glm::vec3(1.0f, 0.5f, 0.31f)),
	m_move_axis(-1)
{}

GameObject::GameObject(const string& mesh_path, const vector<string>& shader_path) : 
	Object(mesh_path, shader_path), m_is_popup(false),
	m_color(glm::vec3(1.0f, 0.5f, 0.31f)), m_move_axis(-1)
{}

GameObject::~GameObject()
{}

void GameObject::move(Camera& camera)
{
	int final_x = 0;
	int final_y = 0;
	float moveCellSize = 0.1f;
	SDL_GetRelativeMouseState(&final_x, &final_y);

	glm::vec3 pos = glm::vec3(0.0, 0.0, 0.0);
	if (m_move_axis == 0)
	{
		if (camera.getForward().z <= 0)
		{
			if (final_x < -1.0)
				pos[m_move_axis] = -moveCellSize;
			else if (final_x > 1.0)
				pos[m_move_axis] = moveCellSize;
		}
		else
		{
			if (final_x > 1.0)
				pos[m_move_axis] = -moveCellSize;
			else if (final_x < -1.0)
				pos[m_move_axis] = moveCellSize;
		}
	}
	else if (m_move_axis == 1)
	{
		if (final_y < -1.0)
			pos[m_move_axis] = moveCellSize;
		else if (final_y > 1.0)
			pos[m_move_axis] = -moveCellSize;
	}
	else
	{
		if (camera.getForward().x >= 0.85f)
		{
			if (final_x < -1.0)
				pos[m_move_axis] = -moveCellSize;
			else if (final_x > 1.0)
				pos[m_move_axis] = moveCellSize;
		}
		else if (camera.getForward().x <= -0.9f)
		{
			if (final_x > 1.0)
				pos[m_move_axis] = -moveCellSize;
			else if (final_x < -1.0)
				pos[m_move_axis] = moveCellSize;
		}
		else if (camera.getForward().z < 0)
		{
			if (final_y < -1.0)
				pos[m_move_axis] = -moveCellSize;
			else if (final_y > 1.0)
				pos[m_move_axis] = moveCellSize;
		}
		else
		{
			if (final_y > 1.0)
				pos[m_move_axis] = -moveCellSize;
			else if (final_y < -1.0)
				pos[m_move_axis] = moveCellSize;
		}
	}
	m_property.at(0) += pos;
	setPosition(m_property.at(0));
}

void GameObject::drawPreview(Material& mat)
{
	glm::mat4 P = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	glm::mat4 V = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 M = glm::mat4(1.0f);

	glm::vec3 dir = { -1.0f, -1.0f, -1.0f };
	glm::vec3 amb = { 1.0f, 1.0f, 1.0f };
	glm::vec3 diff = { 0.8f, 0.8f, 0.8f };
	glm::vec3 spec = { 0.5f, 0.5f, 0.5f };
	unique_ptr<Light> light = make_unique<Light>(dir, amb, diff, spec);
	glm::vec3 view_pos = glm::vec3(0.0f, 0.0f, 3.0f);

	m_shader->load();
	m_shader->setVec3("view_pos", view_pos);
	m_shader->setLight(*light);
	m_shader->setMaterial(mat);
	m_shader->setPVM(P, V, M);
	m_shader->setInt("type", 3);

	m_shader->setInt("shadow_map", 0);
	glActiveTexture(GL_TEXTURE0);
	m_shader->setInt("irradiance_map", 1);
	glActiveTexture(GL_TEXTURE0 + 1);
	m_shader->setInt("prefilter_map", 2);
	glActiveTexture(GL_TEXTURE0 + 2);
	m_shader->setInt("lut_map", 3);
	glActiveTexture(GL_TEXTURE0 + 3);

	if (mat.getTexture() != nullptr)
	{
		m_shader->setInt("type", 1);
		m_shader->setInt("texture_map", 4);
		glActiveTexture(GL_TEXTURE0 + 4);
		mat.getTexture()->setActive();
	}
	m_mesh->draw();

	glActiveTexture(GL_TEXTURE0);
}

void GameObject::drawPreview(vector<shared_ptr<Texture>>& tex)
{
	glm::mat4 P = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	glm::mat4 V = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 M = glm::mat4(1.0f);

	glm::vec3 dir = { -1.0f, -1.0f, -1.0f };
	glm::vec3 amb = { 1.0f, 1.0f, 1.0f };
	glm::vec3 diff = { 0.8f, 0.8f, 0.8f };
	glm::vec3 spec = { 0.5f, 0.5f, 0.5f };
	unique_ptr<Light> light = make_unique<Light>(dir, amb, diff, spec);
	glm::vec3 view_pos = glm::vec3(0.0f, 0.0f, 3.0f);

	m_shader->load();
	m_shader->setVec3("view_pos", view_pos);
	m_shader->setLight(*light);
	m_shader->setPVM(P, V, M);

	m_shader->setInt("shadow_map", 0);
	glActiveTexture(GL_TEXTURE0);
	m_shader->setInt("irradiance_map", 1);
	glActiveTexture(GL_TEXTURE0 + 1);
	m_shader->setInt("prefilter_map", 2);
	glActiveTexture(GL_TEXTURE0 + 2);
	m_shader->setInt("lut_map", 3);
	glActiveTexture(GL_TEXTURE0 + 3);

	m_shader->setInt("type", 1);
	m_shader->setInt("texture_map", 4);
	glActiveTexture(GL_TEXTURE0 + 4);
	tex[0]->setActive();

	m_mesh->draw();

	glActiveTexture(GL_TEXTURE0);
}

void GameObject::draw(const glm::mat4& P, const glm::mat4& V,
	Light& light, glm::vec3& view_pos, ShadowMap& shadow, 
	IrradianceMap& irradiance, PrefilterMap& prefilter, LUTMap& lut)
{
	m_shader->load();
	glm::mat4 shadow_proj = (*shadow.getProj()) * (*shadow.getView());
	m_shader->setMat4("light_matrix", shadow_proj);
	m_shader->setVec3("light_pos", *shadow.getPosition());
	m_shader->setVec3("view_pos", view_pos);
	m_shader->setLight(light);
	m_shader->setInt("preview", 0);

	m_shader->setInt("shadow_map", 0);
	glActiveTexture(GL_TEXTURE0);
	shadow.getBuffer().bindFrameTexture();
	m_shader->setInt("irradiance_map", 1);
	glActiveTexture(GL_TEXTURE0 + 1);
	irradiance.getCubemapBuffer()->bindCubemapTexture();
	m_shader->setInt("prefilter_map", 2);
	glActiveTexture(GL_TEXTURE0 + 2);
	prefilter.getCubemapBuffer()->bindCubemapTexture();
	m_shader->setInt("lut_map", 3);
	glActiveTexture(GL_TEXTURE0 + 3);
	lut.getFrameBuffer()->bindFrameTexture();

	if (m_mesh->getBuffer().getIndices().size() > 1)
		m_mesh->draw(P, V, *m_shader);
	else
		m_mesh->draw(P, V, *m_shader, true);

	//if (m_soft != nullptr)
	//	m_soft->m_tet_mesh->draw(P, V, *m_shader, true);
}

void GameObject::drawInstance(glm::mat4& P, glm::mat4& V)
{
	m_mesh->drawInstance(P, V);
}

Geometry::Geometry() : GameObject()
{}

Geometry::~Geometry()
{}

Point::Point(const vector<info::VertexLayout>& layouts)
{
	m_mesh = make_unique<ParticleMesh>(layouts);

	vector<string> shader_path = { "assets/shaders/Point.vert", "assets/shaders/Point.frag" };
	m_shader = make_unique<Shader>(shader_path);
	m_shader->processShader();
}

Point::~Point() {}

void Point::drawPoint(const glm::mat4& P, const glm::mat4& V)
{	
	m_shader->load();
	glm::mat4 M = glm::mat4(1.0f);
	m_shader->setPVM(P, V, M);

	m_mesh->drawInstance(P, V);
}

Sphere::Sphere(bool is_create_gizmo) :
	Geometry(), m_division(32.0f), m_radius(1.0f)
{
	cout << "********************Create Sphere Constructor********************" << endl;

	m_name = "Sphere";

	vector<info::VertexLayout> layouts = calculateVertex();
	vector<unsigned int> indices = calculateIndex();
	m_mesh = make_shared<Mesh>(m_name);
	m_mesh->getBuffer().createBuffers(layouts, indices);
	m_mesh->computeBoundingBox();

	vector<string> shader_path = { "assets/shaders/BRDF.vert", "assets/shaders/BRDF.frag" };
	m_shader = make_shared<Shader>(shader_path);
	m_shader->processShader();

	cout << "********************Finish create sphere********************\n" << endl;
}

Sphere::~Sphere() 
{
	cout << "Delete Sphere" << endl;
}

vector<info::VertexLayout> Sphere::calculateVertex()
{
	float angle_sector = float(2 * M_PI) / m_division; // 0 to 360
	float angle_stack = float(M_PI / m_division); // 90 to -90

	vector<glm::vec3> vertices;
	vector<glm::vec3> normals;
	vector<glm::vec2> texCoords;

	for (int i = 0; i <= m_division; i++) // stack
	{
		float u = (float)(M_PI / 2) - (float)angle_stack * (float)i;
		for (int j = 0; j <= m_division; j++) // Sector
		{
			float v = (float)j * angle_sector;

			float x = cos(u) * sin(v);
			float y = sin(u);
			float z = cos(u) * cos(v);

			glm::vec3 vertex = { x, y, z };
			vertices.push_back(vertex);

			glm::vec3 normal = { x, y, z };
			normals.push_back(normal);

			float lon = glm::atan(z, x);
			float lat = glm::atan(y, sqrt(x * x + z * z));
			float s = float(lon + M_PI) / float(2 * M_PI);
			float t = float(log(tan(lat / 2 + M_PI / 4)) + M_PI) / float(2 * M_PI);

			glm::vec2 texCoord = {s, t};
			texCoords.push_back(texCoord);
		}
	}

	vector<info::VertexLayout> layouts;
	for (int i = 0; i < vertices.size(); ++i)
	{
		info::VertexLayout layout;
		layout.position = vertices[i]* m_radius;
		layout.normal = normals[i];
		layout.texCoord = texCoords[i];
		layouts.push_back(layout);
	}

	return layouts;
}

vector<unsigned int> Sphere::calculateIndex()
{
	vector<unsigned int> indices;
	unsigned int I1;
	unsigned int I2;

	for (int i = 0; i < m_division; ++i) { // Stack
		I1 = i * unsigned int(m_division + 1);
		I2 = I1 + unsigned int(m_division + 1);
		for (int j = 0; j < m_division; ++j) { // Sector
			if (i != 0)
			{
 				indices.push_back(I1);
				indices.push_back(I2);
				indices.push_back(I1+1);
			}
			if (i != m_division - 1)
			{
				indices.push_back(I1+1);
				indices.push_back(I2);
				indices.push_back(I2+1);
			}
			I1 += 1;
			I2 += 1;
		}
	}

	return indices;
}

Outline::Outline(int width, int height)
{
	cout << "Create an outline constructor" << endl;
	vector<string> outline_shader_paths = { "assets/shaders/Outline.vert", "assets/shaders/Outline.frag" };
	m_outline_shader = make_unique<Shader>(outline_shader_paths);
	m_outline_shader->processShader();

	vector<string> mask_shader_paths = { "assets/shaders/Mask.vert", "assets/shaders/Mask.frag" };
	m_mask_shader = make_unique<Shader>(mask_shader_paths);
	m_mask_shader->processShader();

	for (int i = 0; i < 4; ++i)
	{
		m_outline_buffers.push_back(make_unique<FrameBuffer>());
		m_outline_buffers.at(i)->createBuffers(width, height);
	}

	vector<string> debug_shader = { "assets/shaders/Debug.vert", "assets/shaders/Debug.frag" };
	m_debug = make_unique<GameObject>("assets/models/Debug.txt", debug_shader);
	cout << "Outline finish loading" << endl;
	cout << endl;
}

Outline::~Outline()
{}

void Outline::setupBuffers(GameObject& go, const glm::mat4& V, float width, float height)
{
	if (go.getMesh() == nullptr) return;

	float aspect = width / height;
	glm::mat4 P = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

	glViewport(0, 0, m_outline_buffers[0]->getWidth(), m_outline_buffers[0]->getHeight());

	m_outline_buffers.at(0)->bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_mask_shader->load();  
		if (go.getMesh()->getBuffer().getIndices().size() > 1)
		{
			go.getMesh()->draw(P, V, *m_mask_shader);
		}
		else
		{
			go.getMesh()->draw(P, V, *m_mask_shader, true);
		}
	m_outline_buffers.at(0)->unbind();

	m_debug->setProperty(2, go.getMesh()->getSize());
	m_outline_buffers.at(1)->bind();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_debug->getShader()->load();
		glActiveTexture(GL_TEXTURE0);
		m_outline_buffers.at(0)->bindFrameTexture();
		m_debug->getMesh()->draw();
	m_outline_buffers.at(1)->unbind();

	m_outline_buffers.at(2)->bind();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_outline_shader->load();
		glActiveTexture(GL_TEXTURE0);
		m_outline_buffers.at(1)->bindFrameTexture();
		m_outline_shader->setInt("outline_map", 0);
		m_outline_shader->setFloat("width", 1400);
		m_outline_shader->setFloat("height", 800);
		m_outline_shader->setInt("pass", 0);
		m_outline_shader->setFloat("jump", 1.0f);
		m_debug->getMesh()->draw();
	m_outline_buffers.at(2)->unbind();

	m_outline_buffers.at(3)->bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_outline_shader->load();
		glActiveTexture(GL_TEXTURE0);
		m_outline_buffers.at(2)->bindFrameTexture();
		m_outline_shader->setInt("outline_map", 0);
		m_outline_shader->setFloat("width", 1400);
		m_outline_shader->setFloat("height", 800);
		m_outline_shader->setInt("pass", 0);
		m_outline_shader->setFloat("jump", 2.0f);
		m_debug->getMesh()->draw();
	m_outline_buffers.at(3)->unbind();
}

void Outline::draw(GameObject& go)
{
	m_outline_shader->load();
	glActiveTexture(GL_TEXTURE0);
	m_outline_buffers.back()->bindFrameTexture();
	m_outline_shader->setInt("outline_map", 0);
	glActiveTexture(GL_TEXTURE0+1);
	m_outline_buffers.at(0)->bindFrameTexture();
	m_outline_shader->setInt("mask_map", 1);
	m_outline_shader->setInt("pass", 2);
	m_debug->getMesh()->draw();
}

void Outline::clearOutlineFrame()
{
	m_outline_buffers.back()->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_outline_shader->load();
	m_outline_shader->setInt("pass", 3);
	m_debug->getMesh()->draw();
	m_outline_buffers.back()->unbind();
}

