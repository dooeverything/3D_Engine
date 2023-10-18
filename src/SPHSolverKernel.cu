#include "SPHSolverKernel.cuh"

int n_blocks;
int n_threads;

__device__ glm::ivec3 getHashPos_kernel(const glm::vec3& pos)
{
	return { pos.x / d_params.grid_cell, pos.y / d_params.grid_cell, pos.z / d_params.grid_cell };
}

__device__ uint getHashKey_kernel(const glm::ivec3& pos)
{
	return ((uint)(pos.x * 73856093) ^
			(uint)(pos.y * 19349663) ^
			(uint)(pos.z * 83492791)) % info::HASH_SIZE;
}

//__device__ void copyFromCuda_kernel(int n, glm::vec3* pos, glm::vec3* h_pos)
//{
//	return __device__ void();
//}

void computeBlocks(int n)
{
	n_threads = min(256, n);
	n_blocks = (n % n_threads != 0) ? (n / n_threads + 1) : (n / n_threads);

	cout << "Number of threads: " << n_threads << " Number of blocks: " << n_blocks << endl;
}

cudaError_t setParams(info::SPHParams* params)
{
	cudaError_t cuda_status;
	cuda_status = cudaMalloc((void**)&d_params, sizeof(info::SPHParams));
	if (cuda_status != cudaSuccess)
	{
		cout << "cudaMalloc failed in setParams" << endl;
		return cuda_status;
	}

	cuda_status = cudaMemcpyToSymbol((const void*)&d_params, params, sizeof(info::SPHParams));
	if (cuda_status != cudaSuccess)
	{
		cout << "cudaMemcpy failed in setParams: " << cuda_status << endl;
		return cuda_status;
	}
	
	return cuda_status;
}

cudaError_t setHash(vector<int>& hash, vector<int>& neighbors)
{
	cudaError_t cuda_status;
	cuda_status = cudaMalloc((void**)&d_hash, sizeof(int) * hash.size());
	if (cuda_status != cudaSuccess)
	{
		cout << "cudaMalloc1 failed in setHash" << endl;
		return cuda_status;
	}

	cuda_status = cudaMalloc((void**)&d_neighbors, sizeof(int) * neighbors.size());
	if (cuda_status != cudaSuccess)
	{
		cout << "cudaMalloc2 failed in setHash" << endl;
		return cuda_status;
	}

	cuda_status = cudaMemcpy(d_hash, &hash[0], sizeof(int) * hash.size(), cudaMemcpyHostToDevice);
	if (cuda_status != cudaSuccess)
	{
		cout << "cudaMemcpy1 failed in setHash" << endl;
		return cuda_status;
	}

	cuda_status = cudaMemcpy(d_neighbors, &neighbors[0], sizeof(int) * neighbors.size(), cudaMemcpyHostToDevice);
	if (cuda_status != cudaSuccess)
	{
		cout << "cudaMemcpy2 failed in setHash" << endl;
		return cuda_status;
	}

	return cuda_status;
}

cudaError_t copyToCuda(
	int n,
	vector<glm::vec3>& pos, 
	vector<glm::vec3>& vel,
	vector<glm::vec3>& force,
	vector<float>& density,
	vector<float>& pressure)
{
	cout << "Copy to cuda : " << n << endl;

	cudaError_t cuda_status;
	cuda_status = cudaMalloc((void**)&d_pos, n * sizeof(glm::vec3));
	if (cuda_status != cudaSuccess)
	{
		cout << "cudaMalloc1 failed in copyToCuda " << cuda_status << endl;
		return cuda_status;
	}

	cuda_status = cudaMalloc((void**)&d_velocity, n * sizeof(glm::vec3));
	if (cuda_status != cudaSuccess)
	{
		cout << "cudaMalloc2 failed in copyToCuda" << endl;
		return cuda_status;
	}

	cuda_status = cudaMalloc((void**)&d_force, n * sizeof(glm::vec3));
	if (cuda_status != cudaSuccess)
	{
		cout << "cudaMalloc3 failed in copyToCuda" << endl;
		return cuda_status;
	}

	cuda_status = cudaMalloc((void**)&d_density, n * sizeof(float));
	if (cuda_status != cudaSuccess)
	{
		cout << "cudaMalloc4 failed in copyToCuda" << endl;
		return cuda_status;
	}

	cuda_status = cudaMalloc((void**)&d_pressure, n * sizeof(float));
	if (cuda_status != cudaSuccess)
	{
		cout << "cudaMalloc5 failed in copyToCuda" << endl;
		return cuda_status;
	}

	cuda_status = cudaMemcpy(d_pos, &pos[0], n * sizeof(glm::vec3), cudaMemcpyHostToDevice);
	if (cuda_status != cudaSuccess)
	{
		cout << "cudaMemcpy1 failed while copytocuda position" << endl;
		return cuda_status;
	}

	cuda_status = cudaMemcpy(d_velocity, &vel[0], n * sizeof(glm::vec3), cudaMemcpyHostToDevice);
	if (cuda_status != cudaSuccess)
	{
		cout << "cudaMemcpy2 failed while copytocuda position" << endl;
		return cuda_status;
	}

	cuda_status = cudaMemcpy(d_force, &force[0], n * sizeof(glm::vec3), cudaMemcpyHostToDevice);
	if (cuda_status != cudaSuccess)
	{
		cout << "cudaMemcpy3 failed while copytocuda position" << endl;
		return cuda_status;
	}

	cuda_status = cudaMemcpy(d_density, &density[0], n * sizeof(float), cudaMemcpyHostToDevice);
	if (cuda_status != cudaSuccess)
	{
		cout << "cudaMemcpy4 failed while copytocuda density" << endl;
		return cuda_status;
	}

	cuda_status = cudaMemcpy(d_pressure, &pressure[0], n * sizeof(float), cudaMemcpyHostToDevice);
	if (cuda_status != cudaSuccess)
	{
		cout << "cudaMemcpy5 failed while copytocuda pressure" << endl;
		return cuda_status;
	}

	return cuda_status;
}

cudaError_t copyFromCuda(
	int n,
	vector<glm::vec3>& h_pos)
{
	cudaError_t cuda_status;

	cuda_status = cudaDeviceSynchronize();
	if (cuda_status != cudaSuccess) {
		fprintf(stderr, "cudaDeviceSynchronize returned error code %d after simulation!\n", cuda_status);
		assert(0);
	}

	//glm::vec3* h_pos;
	//cuda_status = cudaMalloc((void**)&h_pos, n * sizeof(glm::vec3));
	//if (cuda_status != cudaSuccess)
	//{
	//	cout << "cudaMalloc1 failed in copyFromCuda " << cuda_status << endl;
	//	assert(0);
	//}


	cuda_status = cudaMemcpy(&h_pos[0], d_pos, n * sizeof(glm::vec3), cudaMemcpyDeviceToHost);
	if (cuda_status != cudaSuccess)
	{
		cout << "cudaMemcpy1 failed in copyFromCuda " << cuda_status << endl;
		return cuda_status;
	}

	//glm::vec3* host_pos = (glm::vec3*)malloc(pos.size() * sizeof(glm::vec3));
	//cout << "Pos size : " << d_pos[0].x << endl;
	//cuda_status = cudaMemcpy(pos, d_pos, pos.size()*sizeof(glm::vec3), cudaMemcpyDeviceToHost);

	cudaFree(d_hash);
	cudaFree(d_neighbors);

	return cuda_status;		
}

cudaError_t simulateCuda(int n, float t, glm::vec3& box, vector<glm::vec3>& pos)
{
	//cout << "SimulateCuda " << n_blocks << " " << n_threads << endl;
	
	cudaError_t cuda_status;

	fillHash_kernel << <n_blocks, n_threads >> > (n, d_hash, d_neighbors, d_pos);
	cuda_status = cudaDeviceSynchronize();
	if (cuda_status != cudaSuccess) {
		fprintf(stderr, "cudaDeviceSynchronize returned error code %d after filling hash!\n", cuda_status);
		assert(0);
	}

	updateDensPress_kernel << <n_blocks, n_threads >> > (n, d_hash, d_neighbors, d_pos, d_density, d_pressure);
	cuda_status = cudaDeviceSynchronize();
	if (cuda_status != cudaSuccess) {
		fprintf(stderr, "cudaDeviceSynchronize returned error code %d after update density and pressure!\n", cuda_status);
		assert( 0);
	}

	updateForce_kernel << <n_blocks, n_threads >> > (n, d_hash, d_neighbors, d_pos, d_velocity, d_force, d_density, d_pressure);
	cuda_status = cudaDeviceSynchronize();
	if (cuda_status != cudaSuccess) {
		fprintf(stderr, "cudaDeviceSynchronize returned error code %d after udpate force!\n", cuda_status);
		assert(0);
	}

	updatePosition_kernel << <n_blocks, n_threads >> > (n, d_pos, d_velocity, d_force, d_density);
	cuda_status = cudaDeviceSynchronize();
	if (cuda_status != cudaSuccess) {
		fprintf(stderr, "cudaDeviceSynchronize returned error code %d after udpate position!\n", cuda_status);
	}

	cuda_status = cudaDeviceSynchronize();
	if (cuda_status != cudaSuccess) {
		fprintf(stderr, "cudaDeviceSynchronize returned error code %d after udpate!\n", cuda_status);
	}

	cuda_status = cudaMemcpy(&pos[0], d_pos, n * sizeof(glm::vec3), cudaMemcpyDeviceToHost);
	if (cuda_status != cudaSuccess)
	{
		cout << "cudaMemcpy1 failed in copyFromCuda " << cuda_status << endl;
		return cuda_status;
	}

	cudaFree(d_hash);
	cudaFree(d_neighbors);

	return cuda_status;
}

cudaError_t freeResources()
{
	//cudaFree(&d_params);
	cudaFree(d_hash);
	cudaFree(d_neighbors);
	cudaFree(d_pos);
	cudaFree(d_velocity);
	cudaFree(d_force);
	cudaFree(d_density);
	cudaFree(d_pressure);

	cudaError_t cuda_status;
	cuda_status = cudaDeviceSynchronize();
	if (cuda_status != cudaSuccess) {
		fprintf(stderr, "cudaDeviceSynchronize returned error code %d after update density and pressure!\n", cuda_status);
		assert(0);
	}
	
	return cuda_status;
}

__global__ void updateDensPress_kernel(
	int n,
	int* hash,
	int* neighbors,
	glm::vec3* pos,
	float* density,
	float* pressure)
{
	int id1 = blockIdx.x * blockDim.x + threadIdx.x;
	if (id1 >= n) return;

	int count = 0;
	float sum = 0.0f;
	glm::vec3 p1 = pos[id1];
	glm::ivec3 p1_grid = getHashPos_kernel(p1);
	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			for (int z = -1; z <= 1; z++)
			{
				glm::ivec3 near_pos = p1_grid + glm::ivec3(x, y, z);
				uint hash_key = getHashKey_kernel(near_pos);
				
				for (int i = hash[hash_key]; i < n * d_params.max_num_neighbors; i += n)
				{
					int id2 = neighbors[i];
					glm::vec3 p2 = pos[id2];

					const float r = glm::length(p2 - p1);
					const float r2 = r * r;
					//if (id1 == 10)
					//{
					//	printf("At %d sum : %f \n", n * d_params.max_num_neighbors, sum);
					//}
					if (r2 < d_params.H2 && id1 != id2)
					{						
						++count;
						float a = pow(d_params.H2 - r2, 3);
						sum += float(d_params.MASS * d_params.POLY6 * a);
						//if (id1 == 10)
						//{
						//	//printf("At %d count : %d, %f = %f * %f * %.10f \n",
						//	//		id1, count, 
						//	//		sum, d_params.MASS, d_params.POLY6, a);

						//	printf("{%.3f %.3f %.3f} - {%.3f %.3f %.3f} \n",
						//			p1.x, p1.y, p1.z, p2.x, p2.y, p2.z);
						//}
					}
				}
			}
		}
	}

	density[id1] = float(d_params.MASS * d_params.POLY6 * pow(d_params.H, 6)) + sum;
	pressure[id1] = d_params.K * (density[id1] - d_params.rDENSITY);

	//if (id1 == 10)
	//{
	//	printf("Final! At %d, Sum : %f density : %f pressure : %f \n", id1, sum, density[id1], pressure[id1]);
	//}
}

__global__ void updateForce_kernel(
	int n,
	int* hash,
	int* neighbors,
	glm::vec3* pos,
	glm::vec3* vel,
	glm::vec3* force,
	float* density,
	float* pressure)
{
	int id1 = blockIdx.x * blockDim.x + threadIdx.x;
	if (id1 >= n) return;

	float sum = 0.0f;
	glm::vec3 p1 = pos[id1];
	glm::ivec3 p1_grid = getHashPos_kernel(p1);
	force[id1] = glm::vec3(0.0f);

	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			for (int z = -1; z <= 1; z++)
			{
				glm::ivec3 near_pos = p1_grid + glm::ivec3(x, y, z);
				uint hash_key = getHashKey_kernel(near_pos);

				for (int i = hash[hash_key]; i < n * d_params.max_num_neighbors; i += n)
				{
					int id2 = neighbors[i];
					glm::vec3 p2 = pos[id2];

					const float r = glm::length(p2 - p1);
					const float r2 = r * r;
					if (r2 < d_params.H2 && id1 != id2)
					{
						glm::vec3 p_dir = glm::normalize(p2 - p1);

						float W = d_params.SPICKY * pow(d_params.H - r, 2);
						float temp = d_params.MASS * (pressure[id1] + pressure[id2]) / (2 * density[id1]);
						glm::vec3 a = -p_dir * temp;
						glm::vec3 f1 = a * W;

						glm::vec3 v_dir = vel[id2] - vel[id1];
						float W2 = d_params.SPICKY2 * (d_params.H - r);
						glm::vec3 b = d_params.VISC * d_params.MASS * (v_dir / density[id2]);
						glm::vec3 f2 = b * W2;

						force[id1] += f1 + f2;

						//if (id1 == 10)
						//{
						//	printf("At %d ", id1);
						//	
						//	printf("p_dir = (%f, %f, %f) = (%f, %f, %f) - (%f, %f, %f) \n", 
						//			p_dir.x, p_dir.y, p_dir.z, 
						//			p1.x, p1.y, p1.z,
						//			p2.x, p2.y, p2.z);

						//	printf("%f = %f * (%f - %f) / (2 * %f) \n",
						//			temp, d_params.MASS, pressure[id1], pressure[id2], density[id1]);

						//	printf("Force : (%f, %f, %f) = (%f, %f, %f) + (%f, %f, %f) \n", 
						//			force[id1].x, force[id1].y, force[id1].z, 
						//			f1.x, f1.y, f1.z, 
						//			f2.x, f2.y, f2.z);
						//}
					}
				}
			}
		}
	}

}

__global__ void updatePosition_kernel(
	int n,
	glm::vec3* pos,
	glm::vec3* vel,
	glm::vec3* force,
	float* density)
{
	int i = blockIdx.x * blockDim.x + threadIdx.x;
	if (i >= n) return;
	
	vel[i] += d_params.t * (force[i] / density[i] + glm::vec3(0.0f, -9.8f, 0.0f));
	pos[i] += d_params.t * vel[i];
	
	//if (i == 10)
	//{
	//	printf("At %d ", i);
	//	
	//	printf("Pos : (%.3f, %.3f, %.3f)  Density : %.3f  Force : (%.3f, %.3f, %.3f) \n",
	//			pos[i].x, pos[i].y, pos[i].z,
	//			density[i],
	//			force[i].x, force[i].y, force[i].z);

	//	printf("Box : (%.3f, %.3f, %.3f) with %f \n", 
	//			d_params.box.x, d_params.box.y, d_params.box.z, d_params.grid_cell);

	//	printf("\n");
	//}
	//	printf("Box : {%.3f, %.3f, %.3f} with H : {%.3f}\n",
	//			d_params.box.x, d_params.box.y, d_params.box.z,
	//			d_params.H);
	//}
	
	if (pos[i].x > -d_params.H + d_params.box.x)
	{
		vel[i].x *= d_params.WALL;
		pos[i].x = -d_params.H + d_params.box.x;
	}
	if (pos[i].x < d_params.H - d_params.box.x)
	{
		vel[i].x *= d_params.WALL;
		pos[i].x = d_params.H - d_params.box.x;
	}

	if (pos[i].y > -d_params.H + d_params.box.y)
	{
		vel[i].y *= d_params.WALL;
		pos[i].y = -d_params.H + d_params.box.y;
	}
	if (pos[i].y < d_params.H)
	{
		vel[i].y *= d_params.WALL;
		pos[i].y = d_params.H;
	}

	if (pos[i].z > -d_params.H + d_params.box.z)
	{
		vel[i].z *= d_params.WALL;
		pos[i].z = -d_params.H + d_params.box.z;
	}
	if (pos[i].z < d_params.H - d_params.box.z)
	{
		vel[i].z *= d_params.WALL;
		pos[i].z = d_params.H - d_params.box.z;
	}
}

__global__ void fillHash_kernel(
	int n, 
	int* hash, 
	int* neighbors, 
	glm::vec3* pos)
{
	int id = blockIdx.x * blockDim.x + threadIdx.x;
	if (id >= n) return;
	
	glm::vec3 p1 = pos[id];
	glm::ivec3 p1_grid = getHashPos_kernel(p1);
	uint hash_key = getHashKey_kernel(p1_grid);

	if (hash[hash_key] == -1)
	{
		hash[hash_key] = id;
		neighbors[id] = id;
	}
	else
	{
		int start = hash[hash_key];
		for (int i = start; i < n * d_params.max_num_neighbors; i += n)
		{
			const float r = glm::length(p1 - pos[start]);
			const float r2 = r * r;
			if (neighbors[i] == -1)
			{
				neighbors[i] = id;
				break;
			}
		}
	}

	//if (hash[hash_key] == 1000)
	//{
	//	printf("Pos : {%.2f, %.2f, %.2f} -> grid_pos : {%d, %d, %d} with hash_key : {%d}\n",
	//	p1.x, p1.y, p1.z, p1_grid.x, p1_grid.y, p1_grid.z, hash_key);
	//	
	//	printf("Hash : {%d}\n", hash[hash_key]);
	//}

}


//void fillHash(int n)
//{
//	fillHash_kernel<<<1, 1>>>(n);
//}
//
//__global__ void fillHash_kernel(int n)
//{
//	int i = blockIdx.x * blockDim.x + threadIdx.x;
//	if (i < n) return;
//
//	//FluidParticle* p = d_particles[i];
//
//	glm::vec3 pos = d_pos[i];
//	glm::ivec3 grid_pos = getHashPos(pos);
//	uint index = getHashKey(grid_pos);
//
//	if (d_hash[index] == nullptr)
//	{
//		d_hash[index] = new FluidParticle(pos);
//		d_hash[index]->m_next = nullptr;
//	}
//	else
//	{
//		FluidParticle* temp = d_hash[index];
//		d_hash[index] = new FluidParticle(pos);
//		d_hash[index]->m_next = temp;
//	}
//}
