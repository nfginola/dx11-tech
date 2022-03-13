# dx11-tech
DX11 project which had the aim of implementing graphics techniques but turned into an architectural playground :)  
This project will likely be a base for any other DX11 related projects I have in the future.
 
There are likely bugs or corner cases not handled, which will be fixed if they ever cause an issue later.
 
There are also things which need to be refactored, but have been ignored since they work for now.
  
Features:
* __Graphics Device API__:
	* Simplified API which mimics Vulkan/DX12
	* Safe strongly typed uin64_t handle-based resources using generational counters
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

* __Techniques (so far)__:
	* Diffuse light ( no specular.. :< )
	* Parallel Split Shadow Maps (PSSM)
	* (paused) Sample Distribution Shadow Maps (SDSM)

* __Miscellaneous__:
	* CPU Profiler (Frame-based and Scoped)
	* GPU Profiler (Frame-based and Scoped)
	* GPU Annotator for clustering commands in a graphics debugger (e.g RenderDoc)
	* ImGUI Docking
	* Assimp Loader
  
Gallery:  
![Alt text](gallery/ss.png?raw=true "Screenshot")
	 



	



