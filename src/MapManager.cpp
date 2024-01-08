#include "MapManager.h"

MapManager::MapManager()
{
	glm::vec3 light_pos = { 1.0f, 1.0f, 1.0f };
	m_depth_map = make_unique<DepthMap>(256, 256);
	m_shadow_map = make_unique<DepthMap>(1024, 1024, light_pos, false);
	m_cubemap = make_unique<CubeMap>(256, 256);
	m_irradiancemap = make_unique<IrradianceMap>(32, 32);
	m_prefilter = make_unique<PrefilterMap>(256, 256);
	m_lut = make_unique<LUTMap>(256, 256);
}

MapManager* MapManager::getManager()
{
	if (m_manager == nullptr)
	{
		m_manager = unique_ptr<MapManager>(new MapManager());
	}

	return m_manager.get();
}

void MapManager::setupPBRMaps()
{
	m_cubemap->drawMap();
	m_irradiancemap->drawMap(*m_cubemap);
	m_prefilter->drawMap(*m_cubemap);
	m_lut->drawMap();
}

void MapManager::setupShadowMap()
{
	m_shadow_map->draw();
}

void MapManager::setupDepthMap(const glm::mat4& P, const glm::mat4& V)
{
	m_depth_map->setProj(P);
	m_depth_map->setView(V);
	m_depth_map->draw();
}

void MapManager::drawCubeMap(const glm::mat4& P, const glm::mat4& V)
{
	m_cubemap->draw(P, V);
}
