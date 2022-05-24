# dx11-tech
DX11 project which had the aim of implementing graphics techniques but turned into an architectural playground :)
This project will likely be a base for any other DX11 related projects I have in the future.
 
There are likely bugs or corner cases not handled, which will be fixed if they ever cause an issue later.
 
There are also things which need to be refactored, but have been ignored since they work for now.
  
Features:
* __Graphics Device API__:
	* Simplified API which mimics Vulkan/DX12 (but still utilizes the slot-binding model)
	* Safe strongly typed uint64_t handle-based resources using generational counters
	* Automated
		* View creation
		* Elimination of redundant re-binds
		* Read-Write and Read resource unbinds
		* MSAA resolves
	* Hot shader recompilation for fast iteration and experimentation
	

* __Graphics API__:
	* Render Queue
		* Bucket-based with int keys
		* Allows for sorting graphics commands in any order (e.g per material, etc.)
	* Reverse Z-depth

* __Techniques__:
	* Deferred Rendering
	* Parallel Split Shadow Maps (PSSM)
		* Utilizing geometry shader instancing
	* Sample Distribution Shadow Maps (SDSM)
		* Parallel Depth Reduction on Compute Shader
		* Depth min/max CPU read-back for frustum subdivision
		* Normal CSM workflow afterwards

	* To-add:
		* Percentage Closer Filtering
		* Normal maps
		* SSAO
		* PBR

* __Miscellaneous__:
	* CPU Profiler (Frame-based and Scoped)
	* GPU Profiler (Frame-based and Scoped)
	* GPU Annotator for clustering commands in a graphics debugger (e.g RenderDoc)
	* ImGUI Docking
	* Assimp Loader
  
* __References__:
	* [PSSM (NVIDIA)](https://developer.nvidia.com/gpugems/gpugems3/part-ii-light-and-shadows/chapter-10-parallel-split-shadow-maps-programmable-gpus/)
	* [SDSM (Intel)](https://www.intel.com/content/www/us/en/developer/articles/technical/sample-distribution-shadow-maps.html)

__Gallery__:  
Scene  
![Alt text](gallery/ss.png?raw=true "Screenshot")  
Exposed graphics primitives  
![Alt text](gallery/ss2.png?raw=true "Screenshot2")  
Setting up depth-only pass example:  
![Alt text](gallery/ss3.png?raw=true "Screenshot3")  
Scoped frame profiler (CPU and GPU) and API usage 
![Alt text](gallery/ss4.png?raw=true "Screenshot4")  
PSSM (Uniform + Logarithmic Split Scheme with Lambda = 0.5)
![Alt text](gallery/PSSM.jpg?raw=true "Screenshot4")  
SDSM (Logarithmic Split Scheme)
![Alt text](gallery/SDSM.jpg?raw=true "Screenshot4")  
	 



	



