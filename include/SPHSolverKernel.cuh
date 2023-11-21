#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <unordered_map>
#include <iostream>
#include <vector>

#include "Utils.h"
//#include "Particle.h"

using namespace std;

// Device

__device__ __constant__ info::SPHParams d_params;
__device__ int* d_hash;
__device__ int* d_neighbors;
__device__ glm::vec3* d_pos;
__device__ glm::vec3* d_velocity;
__device__ glm::vec3* d_force;
__device__ float* d_density;
__device__ float* d_pressure;

__device__ glm::ivec3 getHashPos_kernel(const glm::vec3&);
__device__ info::uint getHashKey_kernel(const glm::ivec3&);

void computeBlocks(int n);
cudaError_t setParams(info::SPHParams* params);
cudaError_t setHash(vector<int>& hash, vector<int>& neighbors);
cudaError_t copyToCuda(
	int n,
	vector<glm::vec3>& pos,
	vector<glm::vec3>& vel,
	vector<glm::vec3>& force,
	vector<float>& density,
	vector<float>& pressure);

cudaError_t copyFromCuda(
	int n,
	vector<glm::vec3>& h_pos);

cudaError_t simulateCuda(int n, float t, vector<glm::vec3>& pos);

cudaError_t freeResources();

__global__ void updateDensPress_kernel(
	int n,
	int* hash,
	int* neighbors,
	glm::vec3* pos,
	float* density,
	float* pressure);

__global__ void updateForce_kernel(
	int n,
	int* hash,
	int* neighbors,
	glm::vec3* pos,
	glm::vec3* vel,
	glm::vec3* force,
	float* density,
	float* pressure);

__global__ void updatePosition_kernel(
	int n,
	glm::vec3* pos,
	glm::vec3* vel,
	glm::vec3* force,
	float* density);

__global__ void fillHash_kernel(
	int n, 
	int* hash,
	int* neighbors,
	glm::vec3* pos);

//void fillHash(int n);
