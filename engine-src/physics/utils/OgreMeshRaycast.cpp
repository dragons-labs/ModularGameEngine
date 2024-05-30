/*
Copyright (c) 2016-2024 Robert Ryszard Paciorek <rrp@opcode.eu.org>

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

Inspired by:
	→ OGRE (MIT licensed)
	→ ogreHTML by Muhammed Ikbal Akpaca (https://bitbucket.org/saejox/ogrehtml/src) (Zlib licensed)
*/

#include "physics/utils/OgreMeshRaycast.h"
#include "physics/utils/OgreColisionBoundingBox.h"

#include <OgreSceneNode.h>
#include <OgreMesh.h>
#include <OgreSubMesh.h>
#include <OgreSubEntity.h>
#include <OgreBillboardSet.h>
#include <OgreMesh2.h>
#include <OgreSubMesh2.h>
#include <Vao/OgreAsyncTicket.h>
#include <Vao/OgreIndexBufferPacked.h>
#include <OgreBitwise.h>

#include "LogSystem.h"

MGE::OgreMeshRaycast::Results MGE::OgreMeshRaycast::entityHitTest(
	Ogre::Ray mouseRay,
	const Ogre::Matrix4& toWorld,
	Ogre::MovableObject* mo,
	const std::vector<Ogre::Vector3>& vertices,
	const std::vector<int>& indices,
	bool positiveSide,
	bool negativeSide
) {
	Ogre::Matrix4 toLocal = toWorld.inverse();
	mouseRay.setOrigin(toLocal * mouseRay.getOrigin());
	toLocal.setTrans(Ogre::Vector3::ZERO);
	mouseRay.setDirection(toLocal * mouseRay.getDirection());
	
	if (indices.size() > 32) {
		// check converted to LOCAL ray and LOCAL AABB (OBB in WORLD space) colision
		Ogre::Aabb aabb = mo->getLocalAabb();
		if (!MGE::OgreColisionBoundingBox::intersects( mouseRay, Ogre::AxisAlignedBox(aabb.getMinimum(), aabb.getMaximum()) ))
			return Results();
	}
	
	return entityHitTest(mouseRay, vertices, indices, positiveSide, negativeSide);
}

MGE::OgreMeshRaycast::Results MGE::OgreMeshRaycast::entityHitTest(
	Ogre::Ray mouseRay,
	const std::vector<Ogre::Vector3>& vertices,
	const std::vector<int>& indices,
	bool positiveSide,
	bool negativeSide
) {
	Results results;
	
	for(int i = 0; i < static_cast<int>(indices.size()); i += 3) {
		std::pair<bool, Ogre::Real> intersectTest = Ogre::Math::intersects(mouseRay, vertices[indices[i]], vertices[indices[i + 1]], vertices[indices[i + 2]], positiveSide, negativeSide);
		
		if (intersectTest.first && (results.index < 0 || results.distance > intersectTest.second)) {
			results.distance = intersectTest.second;
			results.index    = i;
		}
	}
	
	if (results.index >= 0) {
		results.hitPoint = mouseRay.getPoint(results.distance);
	}
	
	return results;
}

Ogre::Vector2 MGE::OgreMeshRaycast::getTexturePoint(
	const Results& hitTest,
	const std::vector<Ogre::Vector3>& vertices,
	const std::vector<int>& indices,
	const std::vector<Ogre::Vector2>& UVs
) {
	Ogre::Vector3 res_v1( vertices[indices[hitTest.index]] );
	Ogre::Vector3 res_v2( vertices[indices[hitTest.index + 1]] );
	Ogre::Vector3 res_v3( vertices[indices[hitTest.index + 2]] );
	
	Ogre::Real Bx, By, Bz;
	//GetBarycentricCoordinates(B, res_v1, res_v2, res_v3, hitTest.hitPoint);
	{
		Ogre::Vector3 v = res_v2 - res_v1;
		Ogre::Vector3 w = res_v3 - res_v1;
		Ogre::Vector3 u = v.crossProduct(w);
		Ogre::Real A = u.length();
		
		v = res_v2 - hitTest.hitPoint;
		w = res_v3 - hitTest.hitPoint;
		u = v.crossProduct(w);
		Bx = u.length() / A;
		
		v = hitTest.hitPoint - res_v1;
		w = res_v3 - res_v1;
		u = v.crossProduct(w);
		By = u.length() / A;
		
		Bz = 1.0f - Bx - By;
	}
	
	#ifdef MGE_DEBUG_MESH_RAYCAST_UV
	LOG_DEBUG("ind " << hitTest.index << " -> " << indices[hitTest.index] << " " << indices[hitTest.index + 1] << " " << indices[hitTest.index + 2]);
	LOG_DEBUG("UVs x: " << UVs[indices[hitTest.index]].x << " " << UVs[indices[hitTest.index+1]].x << " " << UVs[indices[hitTest.index+2]].x);
	LOG_DEBUG("UVs y: " << UVs[indices[hitTest.index]].y << " " << UVs[indices[hitTest.index+1]].y << " " << UVs[indices[hitTest.index+2]].y);
	LOG_DEBUG("B xyz: " << Bx << " " << By << " " << Bz);
	#endif
	
	return UVs[indices[hitTest.index]] * Bx + UVs[indices[hitTest.index + 1]] * By + UVs[indices[hitTest.index + 2]] * Bz;
}

void MGE::OgreMeshRaycast::getMeshInformationV2(
	const Ogre::Item* item,
	std::vector<Ogre::Vector3>* vertices,
	std::vector<int>* indices,
	std::vector<Ogre::Vector2>* UVs,
	bool applyTransform
) {
	Ogre::Mesh* mesh = item->getMesh().get();
	
	Ogre::Vector3 position;
	Ogre::Quaternion orient;
	Ogre::Vector3 scale;
	
	if (applyTransform) {
		Ogre::SceneNode* node = item->getParentSceneNode();
		position = node->_getDerivedPosition();
		orient = node->_getDerivedOrientation();
		scale = node->getScale();
	}
	
	vertices->clear();
	if (UVs) UVs->clear();
	indices->clear();
	
	// Calculate how many vertices and indices we're going to need
	size_t vertex_count = 0;
	size_t index_count = 0;
	
	for(uint16_t m = 0; m < mesh->getNumSubMeshes(); ++m) {
		Ogre::SubMesh* submesh = mesh->getSubMesh(m);
		vertex_count += submesh->mVao[0][0]->getVertexBuffers()[0]->getNumElements();
		index_count  += submesh->mVao[0][0]->getIndexBuffer()->getNumElements();
	}
	
	vertices->resize(vertex_count);
	if (UVs) UVs->resize(vertex_count);
	indices->reserve(index_count);
	
	// Run through the submeshes again, adding the data into the arrays
	size_t vert_offset = 0;
	size_t uv_offset = 0;
	size_t ind_offset = 0;
	for(uint16_t m = 0; m < mesh->getNumSubMeshes(); ++m) {
		Ogre::VertexArrayObject* vao =  mesh->getSubMesh(m)->mVao[Ogre::VpNormal][0];
		
		#if OGRE_VERSION_MAJOR == 2 && OGRE_VERSION_MINOR <= 3
		Ogre::VertexArrayObject::ReadRequestsArray requests;
		#else
		Ogre::VertexArrayObject::ReadRequestsVec requests;
		#endif
		requests.push_back( Ogre::VertexArrayObject::ReadRequests(Ogre::VES_POSITION) );
		if (UVs) {
			requests.push_back( Ogre::VertexArrayObject::ReadRequests(Ogre::VES_TEXTURE_COORDINATES) );
		}
		vao->readRequests( requests );
		vao->mapAsyncTickets( requests );
		
		//
		// VERTICES
		//
		unsigned int vert_num = requests[0].vertexBuffer->getNumElements();
		if (requests[0].type == Ogre::VET_FLOAT3) {
			for (size_t j = 0; j < vert_num; ++j) {
				const float* data = reinterpret_cast<const float*>( requests[0].data );
				
				Ogre::Vector3 posVec(data[0], data[1], data[2]);
				if (applyTransform) {
					(*vertices)[vert_offset + j] = (orient * (posVec * scale)) + position;
				} else {
					(*vertices)[vert_offset + j] = posVec;
				}
				
				requests[0].data += requests[0].vertexBuffer->getBytesPerElement();
			}
		} else if (requests[0].type == Ogre::VET_HALF4) {
			for (size_t j = 0; j < vert_num; ++j) {
				const Ogre::uint16* data = reinterpret_cast<const Ogre::uint16*>( requests[0].data );
				
				Ogre::Vector3 posVec(
					Ogre::Bitwise::halfToFloat(data[0]),
					Ogre::Bitwise::halfToFloat(data[1]),
					Ogre::Bitwise::halfToFloat(data[2])
				);
				if (applyTransform) {
					(*vertices)[vert_offset + j] = (orient * (posVec * scale)) + position;
				} else {
					(*vertices)[vert_offset + j] = posVec;
				}
				
				requests[0].data += requests[0].vertexBuffer->getBytesPerElement();
			}
		} else {
			LOG_WARNING("Unsupported mesh vertex buffor format");
		}
		vert_offset += vert_num;
		
		//
		// TEXTURE UV  - Get vertex UV coordinates
		//
		if (UVs) {
			unsigned int uv_num = requests[1].vertexBuffer->getNumElements();
			if (requests[1].type == Ogre::VET_FLOAT2) {
				for(size_t j = 0; j < uv_num; ++j) {
					const float* data = reinterpret_cast<const float*>( requests[1].data );
					
					(*UVs)[uv_offset + j] = Ogre::Vector2(data[0], data[1]);
					
					#ifdef MGE_DEBUG_MESH_RAYCAST_UV
					LOG_DEBUG("UVs [ " << uv_offset << " + " << j << " ] = (" << data[0] << "; " << data[1] << ")");
					#endif
					
					requests[1].data += requests[1].vertexBuffer->getBytesPerElement();
				}
			} else if (requests[1].type == Ogre::VET_HALF2) {
				for(size_t j = 0; j < uv_num; ++j) {
					const Ogre::uint16* data = reinterpret_cast<const Ogre::uint16*>( requests[1].data );
					
					(*UVs)[uv_offset + j] = Ogre::Vector2(
						Ogre::Bitwise::halfToFloat(data[0]),
						Ogre::Bitwise::halfToFloat(data[1])
					);
					
					#ifdef MGE_DEBUG_MESH_RAYCAST_UV
					LOG_DEBUG("UVs [ " << uv_offset << " + " << j << " ] = (" << Ogre::Bitwise::halfToFloat(data[0]) << "; " << Ogre::Bitwise::halfToFloat(data[1]) << ")");
					#endif
					
					requests[1].data += requests[1].vertexBuffer->getBytesPerElement();
				}
			} else {
				LOG_WARNING("Unsupported mesh UV buffor format");
			}
			uv_offset += uv_num;
			#ifdef MGE_DEBUG_MESH_RAYCAST_UV
			if (vert_num != uv_num)
				LOG_WARNING("vert_num != uv_num");
			#endif
		}
		
		//
		// INDICES
		//
		
		Ogre::IndexBufferPacked* indexBuffer = vao->getIndexBuffer();
		Ogre::AsyncTicketPtr asyncTicket = indexBuffer->readRequest( 0, indexBuffer->getNumElements() );
		
		if (indexBuffer->getIndexType() == Ogre::IndexBufferPacked::IT_32BIT) {
			const uint32_t* pLong  = static_cast<const uint32_t*>(
				asyncTicket->map()
			);
			for(size_t k = 0; k < indexBuffer->getNumElements(); ++k) {
				indices->push_back( pLong[k] + ind_offset );
			}
		} else {
			const uint16_t* pShort = static_cast<const uint16_t*>(
				asyncTicket->map()
			);
			for(size_t k = 0; k < indexBuffer->getNumElements(); ++k) {
				indices->push_back( pShort[k] + ind_offset );
			}
		}
		asyncTicket->unmap();
		ind_offset = vert_offset;
	}
}

void MGE::OgreMeshRaycast::getMeshInformationV1(
	const Ogre::v1::Entity* entity,
	std::vector<Ogre::Vector3>* vertices,
	std::vector<int>* indices,
	std::vector<Ogre::Vector2>* UVs,
	bool applyTransform
) {
	Ogre::v1::Mesh* mesh = entity->getMesh().get();
	
	bool added_shared = false;
	size_t current_offset = 0;
	size_t shared_offset = 0;
	size_t next_offset = 0;
	
	Ogre::Vector3 position;
	Ogre::Quaternion orient;
	Ogre::Vector3 scale;
	
	if (applyTransform) {
		Ogre::SceneNode* node = entity->getParentSceneNode();
		position = node->_getDerivedPosition();
		orient = node->_getDerivedOrientation();
		scale = node->getScale();
	}
	
	vertices->clear();
	if (UVs) UVs->clear();
	
	// Calculate how many vertices and indices we're going to need
	size_t vertex_count = 0;
	size_t index_count = 0;
	
	for(uint16_t m = 0; m < mesh->getNumSubMeshes(); ++m) {
		Ogre::v1::SubMesh* submesh = mesh->getSubMesh(m);
		
		// We only need to add the shared vertices once
		if(submesh->useSharedVertices) {
			if(!added_shared) {
				vertex_count += mesh->sharedVertexData[0]->vertexCount;
				added_shared = true;
			}
		} else {
			vertex_count += submesh->vertexData[0]->vertexCount;
		}
		
		// Add the indices
		index_count += submesh->indexData[0]->indexCount;
	}
	
	added_shared = false;
	
	// Run through the submeshes again, adding the data into the arrays
	for(uint16_t m = 0; m < mesh->getNumSubMeshes(); ++m) {
		Ogre::v1::SubMesh* submesh = mesh->getSubMesh(m);
		
		Ogre::v1::VertexData* vertex_data = submesh->useSharedVertices ? mesh->sharedVertexData[0] : submesh->vertexData[0];
		
		if((!submesh->useSharedVertices) || (submesh->useSharedVertices && !added_shared)) {
			//
			// TEXTURE UV  - Get vertex UV coordinates
			//
			if (UVs) {
				// Get last set of texture coordinates
				int i = 0;
				const Ogre::v1::VertexElement* texcoordElem;
				const Ogre::v1::VertexElement* pCurrentElement = 0;
				do {
					texcoordElem = pCurrentElement;
					pCurrentElement = vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_TEXTURE_COORDINATES, i++);
				} while(pCurrentElement);
				
				Ogre::v1::HardwareVertexBufferSharedPtr vbuf = vertex_data->vertexBufferBinding->getBuffer(texcoordElem->getSource());
				uint8_t* vertex = static_cast<uint8_t*>(vbuf->lock(Ogre::v1::HardwareBuffer::HBL_READ_ONLY));
				
				float* pReal;
				
				UVs->resize(UVs->size() + vertex_data->vertexCount);
				
				for(size_t j = 0; j < vertex_data->vertexCount; ++j, vertex += vbuf->getVertexSize()) {
					texcoordElem->baseVertexPointerToElement(vertex, &pReal);
					(*UVs)[current_offset + j] = Ogre::Vector2(pReal[0], pReal[1]);
					
					#ifdef MGE_DEBUG_MESH_RAYCAST_UV
					LOG_DEBUG("UVs [ " << current_offset << " + " << j << " ] = (" << pReal[0] << "; " << pReal[1] << ")");
					#endif
				}
				
				vbuf->unlock();
			}
			
			//
			// VERTICES
			//
			if(submesh->useSharedVertices) {
				added_shared = true;
				shared_offset = current_offset;
			}
			
			const Ogre::v1::VertexElement* posElem = vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
			Ogre::v1::HardwareVertexBufferSharedPtr vbuf = vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());
			uint8_t* vertex = static_cast<uint8_t*>(vbuf->lock(Ogre::v1::HardwareBuffer::HBL_READ_ONLY));
			
			vertices->resize(vertex_count + vertex_data->vertexCount);
			
			for(size_t j = 0; j < vertex_data->vertexCount; ++j, vertex += vbuf->getVertexSize()) {
				// There is _no_ baseVertexPointerToElement() which takes an Ogre::Real or a double
				//  as second argument. So make it float, to avoid trouble when Ogre::Real will
				//  be comiled/typedefed as double
				float* pReal;
				posElem->baseVertexPointerToElement(vertex, &pReal);
				
				if (applyTransform) {
					(*vertices)[current_offset + j] = (orient * (Ogre::Vector3(pReal[0], pReal[1], pReal[2]) * scale)) + position;
				} else {
					(*vertices)[current_offset + j] = Ogre::Vector3(pReal[0], pReal[1], pReal[2]);
				}
			}
			
			vbuf->unlock();
			next_offset += vertex_data->vertexCount;
		}
		
		//
		// INDICES
		//
		Ogre::v1::IndexData* index_data = submesh->indexData[0];
		size_t offset = (submesh->useSharedVertices) ? shared_offset : current_offset;
		
		indices->clear();
		indices->reserve(indices->size() + index_data->indexCount);
		
		if (index_data->indexBuffer->getType() == Ogre::v1::HardwareIndexBuffer::IT_32BIT) {
			uint32_t* pLong  = static_cast<uint32_t*>(
				index_data->indexBuffer->lock(Ogre::v1::HardwareBuffer::HBL_READ_ONLY)
			);
			for(size_t k = 0; k < index_data->indexCount; ++k) {
				indices->push_back( pLong[k] + offset );
			}
		} else {
			uint16_t* pShort = static_cast<uint16_t*>(
				index_data->indexBuffer->lock(Ogre::v1::HardwareBuffer::HBL_READ_ONLY)
			);
			for(size_t k = 0; k < index_data->indexCount; ++k) {
				indices->push_back( pShort[k] + offset );
			}
		}
		
		index_data->indexBuffer->unlock();
		current_offset = next_offset;
	}
}

#ifndef __DOCUMENTATION_GENERATOR__
namespace MGE {
class BillboardSet : public Ogre::v1::BillboardSet {
public:
	void _getInfo(
		std::vector<Ogre::Vector3>* vertices,
		std::vector<int>* indices,
		std::vector<Ogre::Vector2>* UVs,
		Ogre::Vector3 position
	) const {
		//
		// VERTICES
		//
		if (vertices) {
			const Ogre::v1::VertexElement* posElem = mVertexData->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
			Ogre::v1::HardwareVertexBufferSharedPtr vbuf = mVertexData->vertexBufferBinding->getBuffer(posElem->getSource());
			uint8_t* vertex = static_cast<uint8_t*>(vbuf->lock(Ogre::v1::HardwareBuffer::HBL_READ_ONLY));
			
			vertices->resize(mVertexData->vertexCount);
			
			for(size_t j = 0; j < mVertexData->vertexCount; ++j, vertex += vbuf->getVertexSize()) {
				// There is _no_ baseVertexPointerToElement() which takes an Ogre::Real or a double
				//  as second argument. So make it float, to avoid trouble when Ogre::Real will
				//  be comiled/typedefed as double
				float* pReal;
				posElem->baseVertexPointerToElement(vertex, &pReal);
				
				(*vertices)[j] = Ogre::Vector3(pReal[0], pReal[1], pReal[2]) + position;
			}
			
			vbuf->unlock();
		}
		
		//
		// TEXTURE UV  - Get vertex UV coordinates
		//
		if (UVs) {
			int i = 0;
			const Ogre::v1::VertexElement* texcoordElem;
			const Ogre::v1::VertexElement* pCurrentElement = 0;
			do {
				texcoordElem = pCurrentElement;
				pCurrentElement = mVertexData->vertexDeclaration->findElementBySemantic(Ogre::VES_TEXTURE_COORDINATES, i++);
			} while(pCurrentElement);
			
			Ogre::v1::HardwareVertexBufferSharedPtr vbuf = mVertexData->vertexBufferBinding->getBuffer(texcoordElem->getSource());
			uint8_t* vertex = static_cast<uint8_t*>(vbuf->lock(Ogre::v1::HardwareBuffer::HBL_READ_ONLY));
			
			float* pReal;
			
			UVs->resize(UVs->size() + mVertexData->vertexCount);
			
			for(size_t j = 0; j < mVertexData->vertexCount; ++j, vertex += vbuf->getVertexSize()) {
				texcoordElem->baseVertexPointerToElement(vertex, &pReal);
				(*UVs)[j] = Ogre::Vector2(pReal[0], pReal[1]);
			}
			
			vbuf->unlock();
		}
		
		//
		// INDICES
		//
		if (indices) {
			indices->clear();
			indices->reserve(indices->size() + mIndexData->indexCount);
			
			if (mIndexData->indexBuffer->getType() == Ogre::v1::HardwareIndexBuffer::IT_32BIT) {
				uint32_t* pLong  = static_cast<uint32_t*>(
					mIndexData->indexBuffer->lock(Ogre::v1::HardwareBuffer::HBL_READ_ONLY)
				);
				for(size_t k = 0; k < mIndexData->indexCount; ++k) {
					indices->push_back( pLong[k] );
				}
			} else {
				uint16_t* pShort = static_cast<uint16_t*>(
					mIndexData->indexBuffer->lock(Ogre::v1::HardwareBuffer::HBL_READ_ONLY)
				);
				for(size_t k = 0; k < mIndexData->indexCount; ++k) {
					indices->push_back( pShort[k] );
				}
			}
			
			mIndexData->indexBuffer->unlock();
		}
	}
};
}

void MGE::OgreMeshRaycast::getBillboardInformation(
	const Ogre::v1::BillboardSet* billboardSet,
	std::vector<Ogre::Vector3>* vertices,
	std::vector<int>* indices,
	std::vector<Ogre::Vector2>* UVs,
	const Ogre::Vector3& offset
) {
	const MGE::BillboardSet* _billboardSet = static_cast<const MGE::BillboardSet*>(billboardSet);
	_billboardSet->_getInfo(vertices, indices, UVs, offset);
}
#endif
