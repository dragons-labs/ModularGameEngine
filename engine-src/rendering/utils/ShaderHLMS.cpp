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

#include "rendering/utils/ShaderHLMS.h"

#include "LogSystem.h"

#ifdef MGE_DEBUG_HLMS_SIMPLE_SHADER
#define DEBUG_HLMS_LOG(a) LOG_XDEBUG(a)
#else
#define DEBUG_HLMS_LOG(a)
#endif

#include <OgreRoot.h>
#include <OgreCamera.h>
#include <OgreRenderQueue.h>
#include <Vao/OgreVaoManager.h>
#include <Vao/OgreVertexArrayObject.h>
#include <Vao/OgreConstBufferPacked.h>
#include <Vao/OgreReadOnlyBufferPacked.h>
#include <OgreHlmsManager.h>
#include <OgreHighLevelGpuProgramManager.h>
#include <OgreHighLevelGpuProgram.h>
#include <CommandBuffer/OgreCommandBuffer.h>
#include <CommandBuffer/OgreCbShaderBuffer.h>

MGE::HlmsSimpleShaderDatablock::HlmsSimpleShaderDatablock(
	Ogre::IdString name,
	MGE::HlmsSimpleShader* creator,
	const Ogre::HlmsMacroblock* macroblock,
	const Ogre::HlmsBlendblock* blendblock,
	const Ogre::HlmsParamVec& params
) :
	HlmsDatablock(name, creator, macroblock, blendblock, params),
	colorRed(1.0f), colorGreen(1.0f), colorBlue(1.0f), colorAlpha(1.0f), lineWidth(0.0f)
{
	LOG_DEBUG("HlmsDatablock:: constructor");
	creator->requestSlot( 0, this, false );
}

MGE::HlmsSimpleShaderDatablock::~HlmsSimpleShaderDatablock() {
	LOG_DEBUG("HlmsDatablock:: destructor");
	if( mAssignedPool )
		static_cast<MGE::HlmsSimpleShader*>(mCreator)->releaseSlot( this );
}

void MGE::HlmsSimpleShaderDatablock::uploadToConstBuffer( char* dstPtr, Ogre::uint8 dirtyFlags ) {
	memcpy( dstPtr, &colorRed, MaterialSizeInGpu );
}

void MGE::HlmsSimpleShaderDatablock::setPrograms(Ogre::HlmsPso& pso, Ogre::uint32 hash) const {
	LOG_DEBUG("HlmsSimpleShaderDatablock::setPrograms");
	
	Ogre::HighLevelGpuProgramManager* gpuProgMgr = Ogre::HighLevelGpuProgramManager::getSingletonPtr();
	
	const Ogre::String ShaderFiles[] = { "VertexShader_vs", "PixelShader_ps", "GeometryShader_gs", "HullShader_hs", "DomainShader_ds" };
	for (int i=0; i<5; ++i) {
		Ogre::String filename = shaderName + "_" + ShaderFiles[i] + "." + "glsl";
		
		if (Ogre::ResourceGroupManager::getSingleton().findResourceFileInfo( "MGE_Programs4GPU", filename )->size() > 0) {
			Ogre::HighLevelGpuProgramPtr gp = gpuProgMgr->createProgram(
				Ogre::StringConverter::toString( hash ) + ShaderFiles[i], "MGE_Programs4GPU", "glsl", static_cast<Ogre::GpuProgramType>(i)
			);
			LOG_DEBUG("load " << filename);
			gp->setSourceFile( filename );
			gp->load();
			
			switch(i) {
				case 0:
					pso.vertexShader = gp;
					break;
				case 2:
					pso.geometryShader = gp;
					break;
				case 3:
					pso.tesselationHullShader = gp;
					break;
				case 4:
					pso.tesselationDomainShader = gp;
					break;
				case 1:
					pso.pixelShader = gp;
					break;
			}
		} else {
			if (i<2) {
				LOG_WARNING("can't find gpu program file: " << filename);
			} else {
				LOG_INFO("can't find gpu program file: " << filename);
			}
		}
	}
}



const Ogre::HlmsTypes MGE::HlmsSimpleShader::TYPE = Ogre::HLMS_USER1;

MGE::HlmsSimpleShader::HlmsSimpleShader() :
	HlmsBufferManager(TYPE, "SimpleShader", 0, 0),
	ConstBufferPool(MGE::HlmsSimpleShaderDatablock::MaterialSizeInGpuAligned(), Ogre::ConstBufferPool::ExtraBufferParams()),
	mLastBoundPool(NULL)
{
	#ifdef MGE_DEBUG
	setDebugOutputPath(true, true, MGE_DEBUG_HLMS_PATH);
	#else
	setDebugOutputPath(false, false);
	#endif
	Ogre::Root::getSingleton().getHlmsManager()->registerHlms(this);
}

MGE::HlmsSimpleShader::~HlmsSimpleShader() {
}

MGE::HlmsSimpleShaderDatablock* MGE::HlmsSimpleShader::getOrCreateDatablock(const Ogre::String& datablockName, const Ogre::String& gpuProgsBasename, const Ogre::ColourValue& color, float lineWidth) {
	MGE::HlmsSimpleShaderDatablock* datablock = static_cast<MGE::HlmsSimpleShaderDatablock*>(
		Ogre::Root::getSingletonPtr()->getHlmsManager()->getDatablockNoDefault(datablockName)
	);
	if (!datablock) {
		auto hlms = MGE::HlmsSimpleShader::getPtr(); /* == Ogre::Root::getSingletonPtr()->getHlmsManager()->getHlms(MGE::HlmsSimpleShader::TYPE) but we need create singleton when not exists before ...*/
		datablock = static_cast<MGE::HlmsSimpleShaderDatablock*>(
			hlms->createDatablock(
				datablockName, datablockName,
				Ogre::HlmsMacroblock(), Ogre::HlmsBlendblock(), Ogre::HlmsParamVec()
			)
		);
	}
	
	datablock->shaderName = gpuProgsBasename;
	datablock->colorRed = color.r;
	datablock->colorGreen = color.g;
	datablock->colorBlue = color.b;
	datablock->colorAlpha = color.a;
	datablock->lineWidth = lineWidth;
	
	return datablock;
}

Ogre::HlmsDatablock* MGE::HlmsSimpleShader::createDatablockImpl(
	Ogre::IdString datablockName,
	const Ogre::HlmsMacroblock* macroblock,
	const Ogre::HlmsBlendblock* blendblock,
	const Ogre::HlmsParamVec& paramVec
) {
	return OGRE_NEW MGE::HlmsSimpleShaderDatablock( datablockName, this, macroblock, blendblock, paramVec );
}

Ogre::uint32 MGE::HlmsSimpleShader::fillBuffersFor(
		const Ogre::HlmsCache* cache,
		const Ogre::QueuedRenderable& queuedRenderable,
		bool casterPass, Ogre::uint32 lastCacheHash,
		Ogre::uint32 lastTextureHash
) {
	OGRE_EXCEPT( Ogre::Exception::ERR_NOT_IMPLEMENTED,
		"Trying to use slow-path on a desktop implementation. "
		"Change the RenderQueue settings.",
		"MGE::HlmsSimpleShader::fillBuffersFor"
	);
}

Ogre::uint32 MGE::HlmsSimpleShader::fillBuffersForV1(
	const Ogre::HlmsCache* cache,
	const Ogre::QueuedRenderable& queuedRenderable,
	bool casterPass, Ogre::uint32 lastCacheHash,
	Ogre::CommandBuffer* commandBuffer
) {
	return fillBuffersForV2( cache, queuedRenderable, casterPass, lastCacheHash, commandBuffer );
}

void MGE::HlmsSimpleShader::calculateHashFor(
	Ogre::Renderable* renderable,
	Ogre::uint32& outHash, Ogre::uint32& outCasterHash
) {
	DEBUG_HLMS_LOG("Hlms::calculateHashFor");
	
	return Ogre::HlmsBufferManager::calculateHashFor(renderable, outHash, outCasterHash);
}

const Ogre::HlmsCache* MGE::HlmsSimpleShader::createShaderCacheEntry(
	Ogre::uint32 renderableHash,
	const Ogre::HlmsCache& passCache,
	Ogre::uint32 finalHash,
	const Ogre::QueuedRenderable& queuedRenderable
	#if defined (OGRE_NEXT_VERSION) && OGRE_NEXT_VERSION >= 0x40000
	, Ogre::HlmsCache *reservedStubEntry, size_t threadIdx
	#endif
) {
	DEBUG_HLMS_LOG("Hlms::createShaderCacheEntry");
	
	Ogre::Renderable* renderable = queuedRenderable.renderable;
	const MGE::HlmsSimpleShaderDatablock* datablock = static_cast<const MGE::HlmsSimpleShaderDatablock*>(renderable->getDatablock());
	
	// prepare HlmsPso (including loading GPU program) for renderable and its datablock
	Ogre::HlmsPso pso;
	pso.initialize();
	datablock->setPrograms(pso, finalHash); // << here load and compile shaders
	bool casterPass = false;
	
	pso.macroblock = datablock->getMacroblock( casterPass );
	pso.blendblock = datablock->getBlendblock( casterPass );
	pso.pass = passCache.pso.pass;
	
	if( renderable ) {
		const Ogre::VertexArrayObjectArray& vaos = renderable->getVaos( static_cast<Ogre::VertexPass>(casterPass) );
		if( !vaos.empty() ) {
			//v2 object.
			pso.operationType = vaos.front()->getOperationType();
			pso.vertexElements = vaos.front()->getVertexDeclaration();
		} else {
			//v1 object.
			Ogre::v1::RenderOperation renderOp;
			renderable->getRenderOperation( renderOp, casterPass );
			pso.operationType = renderOp.operationType;
			pso.vertexElements = renderOp.vertexData->vertexDeclaration->convertToV2();
		}
		
		pso.enablePrimitiveRestart = true;
	}
	
	applyStrongMacroblockRules( pso );
	
	mRenderSystem->_hlmsPipelineStateObjectCreated( &pso );
	
	const Ogre::HlmsCache* retVal = addShaderCache( finalHash, pso );
	return retVal;
}

Ogre::HlmsCache MGE::HlmsSimpleShader::preparePassHash(
	const Ogre::CompositorShadowNode* shadowNode,
	bool casterPass, bool dualParaboloid,
	Ogre::SceneManager* sceneManager
) {
	DEBUG_HLMS_LOG("Hlms::preparePassHash");
	
	// prepare HlmsCache (see Ogre::Hlms::preparePassHash)
	PassCache passCache;
	passCache.passPso = getPassPsoForScene( sceneManager );
	
	PassCacheVec::iterator it = std::find( mPassCache.begin(), mPassCache.end(), passCache );
	if( it == mPassCache.end() ) {
		mPassCache.push_back( passCache );
		it = mPassCache.end() - 1;
	}
	
	const Ogre::uint32 hash = (it - mPassCache.begin()) << Ogre::HlmsBits::PassShift;
	
	Ogre::HlmsCache retVal( hash, mType, Ogre::HlmsPso() );
	retVal.setProperties = mT[kNoTid].setProperties;
	retVal.pso.pass = passCache.passPso;
	
	// prepare buffers (see Ogre::HlmsBufferManager::preparePassHash)
	if( mTexBuffers.empty() ) {
		size_t bufferSize = std::min<size_t>(
			mTextureBufferDefaultSize, mVaoManager->getTexBufferMaxSize()
		);
		Ogre::ReadOnlyBufferPacked* newBuffer = static_cast<Ogre::ReadOnlyBufferPacked*>(
			mVaoManager->createTexBuffer(
				Ogre::PFG_RGBA32_FLOAT, bufferSize, Ogre::BT_DYNAMIC_PERSISTENT, 0, false
			)
		);
		mTexBuffers.push_back( newBuffer );
	}
	
	// get projectionMatrix and viewMatrix from camera
	const Ogre::Camera* camera = sceneManager->getCamerasInProgress().renderingCamera;
	const Ogre::Matrix4& projectionMatrix = camera->getProjectionMatrixWithRSDepth();
	const Ogre::Matrix4& viewMatrix       = camera->getViewMatrix(true);
	
	// prepare viewProjectionMatrix and identityProjectionMatrix
	viewProjectionMatrix = projectionMatrix * viewMatrix;
	mRenderSystem->_convertProjectionMatrix( Ogre::Matrix4::IDENTITY, identityProjectionMatrix );
	
	// (if need) expose to GPU data constants for pass (e.g. projectionMatrix, viewMatrix)
	
	// expose materials info
	ConstBufferPool::uploadDirtyDatablocks();
	
	return retVal;
}

Ogre::uint32 MGE::HlmsSimpleShader::fillBuffersForV2(
	const Ogre::HlmsCache* cache,
	const Ogre::QueuedRenderable& queuedRenderable,
	bool casterPass, Ogre::uint32 lastCacheHash,
	Ogre::CommandBuffer* commandBuffer
) {
	DEBUG_HLMS_LOG("Hlms::fillBuffersForV*");
	
	Ogre::Renderable* renderable = queuedRenderable.renderable;
	const MGE::HlmsSimpleShaderDatablock* datablock = static_cast<const MGE::HlmsSimpleShaderDatablock*>(renderable->getDatablock());
	
	
	if( OGRE_EXTRACT_HLMS_TYPE_FROM_CACHE_HASH( lastCacheHash ) != mType ) {
		// We changed HlmsType, rebind the shared textures.
		mLastBoundPool = 0;
		
		//layout(binding = 2) uniform InstanceBuffer {} instance
		if( mCurrentConstBuffer < mConstBuffers.size() && static_cast<size_t>((mCurrentMappedConstBuffer - mStartMappedConstBuffer) + 4) <= mCurrentConstBufferSize ) {
			*commandBuffer->addCommand<Ogre::CbShaderBuffer>() = Ogre::CbShaderBuffer(
				Ogre::PixelShader, 2, mConstBuffers[mCurrentConstBuffer], 0, 0
			);
		}
		rebindTexBuffer( commandBuffer );
	}
	
	if( mLastBoundPool != datablock->getAssignedPool() && !casterPass ) {
		//layout(binding = 1) uniform MaterialBuf {} materialArray
		const Ogre::ConstBufferPool::BufferPool* newPool = datablock->getAssignedPool();
		*commandBuffer->addCommand<Ogre::CbShaderBuffer>() = Ogre::CbShaderBuffer(
			Ogre::PixelShader, 1, newPool->materialBuffer, 0, newPool->materialBuffer->getTotalSizeBytes()
		);
		
		mLastBoundPool = newPool;
	}
	
	// prepare InstanceBuffer {} instance buffor
	Ogre::uint32 * RESTRICT_ALIAS currentMappedConstBuffer = mCurrentMappedConstBuffer;
	if (static_cast<size_t>((currentMappedConstBuffer - mStartMappedConstBuffer) + 4) > mCurrentConstBufferSize) {
		currentMappedConstBuffer = mapNextConstBuffer( commandBuffer );
	}
	
	// expose materialId to InstanceBuffer
	*currentMappedConstBuffer = datablock->getAssignedSlot();
	currentMappedConstBuffer += 4;
	
	
	// prepare WorldViewProjection Matrix
	bool useIdentityProjection = queuedRenderable.renderable->getUseIdentityProjection();
	const Ogre::Matrix4& worldMat = queuedRenderable.movableObject->_getParentNodeFullTransform();
	Ogre::Matrix4 worldViewProjMat;
	if (useIdentityProjection) {
		worldViewProjMat = identityProjectionMatrix * worldMat;
	} else {
		worldViewProjMat = viewProjectionMatrix * worldMat;
	}
	
	// prepare buffor for (constant for this renderable) input data for shader (WorldViewProjection Matrix)
	const size_t minimumTexBufferSize = 16;
	float * RESTRICT_ALIAS currentMappedTexBuffer = mCurrentMappedTexBuffer;
	if( currentMappedTexBuffer - mStartMappedTexBuffer + minimumTexBufferSize >= mCurrentTexBufferSize ) {
		mapNextTexBuffer( commandBuffer, minimumTexBufferSize * sizeof(float) );
		currentMappedTexBuffer = mCurrentMappedTexBuffer;
	}
	
	// expose WorldViewProjection Matrix
	#if !OGRE_DOUBLE_PRECISION
	memcpy( currentMappedTexBuffer, &worldViewProjMat, sizeof( Ogre::Matrix4 ) );
	currentMappedTexBuffer += 16;
	#else
	for( int y = 0; y < 4; ++y )
		for( int x = 0; x < 4; ++x )
			*currentMappedTexBuffer++ = worldViewProjMat[ y ][ x ];
	#endif
	
	
	// update mCurrentMappedConstBuffer and mCurrentMappedTexBuffer
	mCurrentMappedConstBuffer = currentMappedConstBuffer;
	mCurrentMappedTexBuffer = currentMappedTexBuffer;
	
	// calcluate and return drawID
	Ogre::uint32 drawID = ((mCurrentMappedTexBuffer - mRealStartMappedTexBuffer) >> 4) - 1;
	DEBUG_HLMS_LOG(" drawID = " << drawID);
	return drawID;
}

void MGE::HlmsSimpleShader::_changeRenderSystem( Ogre::RenderSystem* newRs ) {
	DEBUG_HLMS_LOG("Hlms::_changeRenderSystem");
	
	if( mVaoManager )
		destroyAllBuffers();
	
	ConstBufferPool::_changeRenderSystem( newRs );
	HlmsBufferManager::_changeRenderSystem( newRs );
	
	if( newRs ) {
		HlmsDatablockMap::const_iterator itor = mDatablocks.begin();
		HlmsDatablockMap::const_iterator end  = mDatablocks.end();
		
		while( itor != end ) {
			MGE::HlmsSimpleShaderDatablock* datablock = static_cast<MGE::HlmsSimpleShaderDatablock*>( itor->second.datablock );
		
			ConstBufferPool::requestSlot( 0, datablock, false );
			++itor;
		}
	}
}

void MGE::HlmsSimpleShader::setupRootLayout(
	Ogre::RootLayout& rootLayout
	#if defined (OGRE_NEXT_VERSION) && OGRE_NEXT_VERSION >= 0x40000
	, size_t tid
	#endif
) { }

void MGE::HlmsSimpleShader::frameEnded(void) {
	DEBUG_HLMS_LOG("Hlms::frameEnded");
	HlmsBufferManager::frameEnded();
}
