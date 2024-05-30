/*
Copyright (c) 2017-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>
Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Based on:
	→ OGRE (MIT licensed)
*/

#pragma   once
#include "config.h"

#include "BaseClasses.h"

#include <OgreMatrix4.h>
#include <OgreHlmsDatablock.h>
#include <OgreConstBufferPool.h>
#include <OgreHlmsBufferManager.h>

namespace MGE {

/// @addtogroup Rendering
/// @{
/// @file

class HlmsSimpleShader;

/**
 * @brief HMLS shader datablock
 */
struct HlmsSimpleShaderDatablock : public Ogre::HlmsDatablock, public Ogre::ConstBufferPoolUser {
	Ogre::String shaderName;
	float colorRed;
	float colorGreen;
	float colorBlue;
	float colorAlpha;
	float lineWidth;
	
	HlmsSimpleShaderDatablock(
		Ogre::IdString name,
		HlmsSimpleShader* creator,
		const Ogre::HlmsMacroblock* macroblock,
		const Ogre::HlmsBlendblock* blendblock,
		const Ogre::HlmsParamVec& params
	);
	virtual ~HlmsSimpleShaderDatablock();
	
	void uploadToConstBuffer( char* dstPtr, Ogre::uint8 dirtyFlags ) override;
	
	void setPrograms(Ogre::HlmsPso& pso, Ogre::uint32 hash) const;
	
	static constexpr size_t MaterialSizeInGpu = 5 * sizeof(float);
	static size_t MaterialSizeInGpuAligned() {
		static size_t size = Ogre::alignToNextMultiple(
			MaterialSizeInGpu, 4 * sizeof(float)
		);
		return size;
	}
};

/**
 * @brief HLMS shader
 */
class HlmsSimpleShader : public Ogre::HlmsBufferManager, public Ogre::ConstBufferPool, public MGE::TrivialSingleton<HlmsSimpleShader> {
protected:
	Ogre::Matrix4 viewProjectionMatrix;
	Ogre::Matrix4 identityProjectionMatrix;
	
	Ogre::ConstBufferPool::BufferPool const* mLastBoundPool;
	
	Ogre::HlmsDatablock* createDatablockImpl(
		Ogre::IdString datablockName,
		const Ogre::HlmsMacroblock* macroblock,
		const Ogre::HlmsBlendblock* blendblock,
		const Ogre::HlmsParamVec& paramVec
	) override;
	
	friend class TrivialSingleton;
	HlmsSimpleShader();
	
public:
	virtual ~HlmsSimpleShader();
	
	static const Ogre::HlmsTypes TYPE;
	
	void _changeRenderSystem( Ogre::RenderSystem* newRs ) override;
	
	/// Called by the renderable when either it changes the material, or its properties change (e.g. the mesh' uvs are stripped)
	virtual void calculateHashFor(
		Ogre::Renderable* renderable,
		Ogre::uint32& outHash, Ogre::uint32& outCasterHash
	) override;
	
	/// Creates a shader based on input parameters
	virtual const Ogre::HlmsCache* createShaderCacheEntry(
		Ogre::uint32 renderableHash,
		const Ogre::HlmsCache& passCache,
		Ogre::uint32 finalHash,
		const Ogre::QueuedRenderable& queuedRenderable
		#if defined (OGRE_NEXT_VERSION) && OGRE_NEXT_VERSION >= 0x40000
		, Ogre::HlmsCache *reservedStubEntry, size_t threadIdx
		#endif
	) override;
	
	/// Called every frame by the Render Queue to cache the properties needed by this pass. i.e. Number of PSSM splits, number of shadow casting lights, etc
	virtual Ogre::HlmsCache preparePassHash(
		const Ogre::CompositorShadowNode* shadowNode,
		bool casterPass, bool dualParaboloid,
		Ogre::SceneManager* sceneManager
	) override;
	
	
	/// Fills the constant buffers. Gets executed right before drawing the mesh.
	virtual Ogre::uint32 fillBuffersForV2(
		const Ogre::HlmsCache* cache,
		const Ogre::QueuedRenderable& queuedRenderable,
		bool casterPass, Ogre::uint32 lastCacheHash,
		Ogre::CommandBuffer* commandBuffer
	) override;
	
	virtual Ogre::uint32 fillBuffersForV1(
		const Ogre::HlmsCache* cache,
		const Ogre::QueuedRenderable& queuedRenderable,
		bool casterPass, Ogre::uint32 lastCacheHash,
		Ogre::CommandBuffer* commandBuffer
	) override;
	
	[[ noreturn ]] virtual Ogre::uint32 fillBuffersFor(
		const Ogre::HlmsCache* cache,
		const Ogre::QueuedRenderable& queuedRenderable,
		bool casterPass, Ogre::uint32 lastCacheHash,
		Ogre::uint32 lastTextureHash
	) override;
	
	/// Called when the frame has fully ended (ALL passes have been executed to all RTTs)
	void frameEnded() override;
	
	void setupRootLayout(
		Ogre::RootLayout& rootLayout
		#if defined (OGRE_NEXT_VERSION) && OGRE_NEXT_VERSION >= 0x40000
		, size_t tid
		#endif
	) override;
	
	/**
	 * @brief get or (if don't exist) create HlmsSimpleShaderDatablock
	 * 
	 * @param datablockName     name of created datablock
	 * @param gpuProgsBasename  base name for GPU programs files (to this name will be added "_VertexShader_vs.glsl", "_PixelShader_ps.glsl", "_GeometryShader_gs.glsl", "_HullShader_hs.glsl" or "_DomainShader_ds.glsl")
	 * @param color             color value for shader
	 * @param lineWidth         line width value for shader
	 */
	static HlmsSimpleShaderDatablock* getOrCreateDatablock(const Ogre::String& datablockName, const Ogre::String& gpuProgsBasename, const Ogre::ColourValue& color, float lineWidth);
};

/// @}

}
