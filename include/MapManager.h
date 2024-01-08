#pragma once
#ifndef MAPMANAGER_H
#define MAPMANAGER_H

#include <iostream>
#include <vector>
#include "Map.h"

class MapManager
{
public:
	MapManager(MapManager const&) = delete;
	MapManager& operator=(MapManager const&) = delete;

	static MapManager* getManager();

	void setupPBRMaps();
	void setupShadowMap();
	void setupDepthMap(const glm::mat4& P, const glm::mat4& V);
	void drawCubeMap(const glm::mat4& P, const glm::mat4& V);

	inline glm::mat4 getShadowMapProj() { return m_shadow_map->getProj(); };
	inline glm::mat4 getShadowMapView() { return m_shadow_map->getView(); };
	inline glm::vec3 getShadowMapPos() { return m_shadow_map->getPosition(); };

	inline void bindShadowMap() { m_shadow_map->bindTexture(); };
	inline void bindDepthmap() { m_depth_map->bindTexture(); };
	inline void bindIrradianceMap() { m_irradiancemap->bindTexture(); };
	inline void bindPrefilterMap() { m_prefilter->bindTexture(); };
	inline void bindLUTMap() { m_lut->bindTexture(); };

private:
	unique_ptr<DepthMap> m_shadow_map;
	unique_ptr<DepthMap> m_depth_map;

	// Maps for PBR 
	unique_ptr<CubeMap> m_cubemap;
	unique_ptr<IrradianceMap> m_irradiancemap;
	unique_ptr<PrefilterMap> m_prefilter;
	unique_ptr<LUTMap> m_lut;

	static unique_ptr<MapManager> m_manager;
	MapManager();
	
};

#endif // !MAPMANAGER_H
