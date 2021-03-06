/** 
 * @file llvotextbubble.cpp
 * @brief Viewer-object text bubble.
 *
 * $LicenseInfo:firstyear=2001&license=viewergpl$
 * 
 * Copyright (c) 2001-2009, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlifegrid.net/programs/open_source/licensing/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at
 * http://secondlifegrid.net/programs/open_source/licensing/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 */

#include "llviewerprecompiledheaders.h"

#include "llvotextbubble.h"

#include "imageids.h"
#include "llviewercontrol.h"
#include "llprimitive.h"
#include "llrendersphere.h"

#include "llagent.h"
#include "llbox.h"
#include "lldrawable.h"
#include "llface.h"
#include "llviewertexturelist.h"
#include "llvolume.h"
#include "pipeline.h"
#include "llvector4a.h"
#include "llviewerregion.h"

LLVOTextBubble::LLVOTextBubble(const LLUUID &id, const LLPCode pcode,
							   LLViewerRegion *regionp)
:	LLAlphaObject(id, pcode, regionp)
{
	setScale(LLVector3(1.5f, 1.5f, 0.25f));
	mbCanSelect = FALSE;
	mLOD = MIN_LOD;
	mVolumeChanged = TRUE;
	setVelocity(LLVector3(0.f, 0.f, 0.75f));
	LLVolumeParams volume_params;
	volume_params.setType(LL_PCODE_PROFILE_CIRCLE, LL_PCODE_PATH_LINE);
	volume_params.setBeginAndEndS(0.f, 1.f);
	volume_params.setBeginAndEndT(0.f, 1.f);
	volume_params.setRatio(0.25f, 0.25f);
	volume_params.setShear(0.f, 0.f);
	setVolume(volume_params, 0);
	mColor = LLColor4(1.0f, 0.0f, 0.0f, 1.f);
	for (S32 i = 0, count = getNumTEs(); i < count; i++)
	{
		setTEColor(i, mColor);
		setTETexture(i, LLUUID(IMG_DEFAULT));
	}
}

LLVOTextBubble::~LLVOTextBubble()
{
}

BOOL LLVOTextBubble::isActive() const
{
	return TRUE;
}

BOOL LLVOTextBubble::idleUpdate(LLAgent &agent, LLWorld	&world, const F64 &time)
{
	F32 dt = mUpdateTimer.getElapsedTimeF32();
	// Die after a few seconds.
	if (dt > 1.5f)
	{
		return FALSE;
	}

	LLViewerObject::idleUpdate(agent, world, time);

	setScale(0.5f * (1.f+dt) * LLVector3(1.5f, 1.5f, 0.5f));

	F32 alpha = 0.35f*dt;

	LLColor4 color = mColor;
	color.mV[VALPHA] -= alpha;
	if (color.mV[VALPHA] <= 0.05f)
	{
		return FALSE;
	}
	for (S32 i = 0, count = getNumTEs(); i < count; i++)
	{
		setTEColor(i, color);
		setTEFullbright(i, TRUE);
	}

	gPipeline.markRebuild(mDrawable, LLDrawable::REBUILD_VOLUME, TRUE);
	return TRUE;
}

void LLVOTextBubble::updateTextures()
{
	// Update the image levels of all textures...

	for (U32 i = 0, count = getNumTEs(); i < count; i++)
	{
		const LLTextureEntry *te = getTE(i);
		F32 texel_area_ratio = fabs(te->mScaleS * te->mScaleT);
		texel_area_ratio = llclamp(texel_area_ratio, .125f, 16.f);
		LLViewerTexture* imagep = getTEImage(i);
		if (imagep)
		{
			imagep->addTextureStats(mPixelArea / texel_area_ratio);
		}
	}
}

LLDrawable *LLVOTextBubble::createDrawable(LLPipeline *pipeline)
{
	pipeline->allocDrawable(this);
	mDrawable->setLit(FALSE);
	mDrawable->setRenderType(LLPipeline::RENDER_TYPE_VOLUME);
	
	for (U32 i = 0, count = getNumTEs(); i < count; i++)
	{
		LLViewerTexture *imagep;
		const LLTextureEntry *texture_entry = getTE(i);
		imagep = LLViewerTextureManager::getFetchedTexture(texture_entry->getID());

		mDrawable->addFace((LLFacePool*) NULL, imagep);
	}

	return mDrawable;
}

// virtual
BOOL LLVOTextBubble::setVolume(const LLVolumeParams &volume_params, const S32 detail, bool unique_volume)
{
	if (LLPrimitive::setVolume(volume_params, mLOD))
	{
		if (mDrawable)
		{
			gPipeline.markRebuild(mDrawable, LLDrawable::REBUILD_VOLUME, TRUE);
			mVolumeChanged = TRUE;
		}
		return TRUE;
	}
	return FALSE;
}


BOOL LLVOTextBubble::updateLOD()
{
	return FALSE;
}

BOOL LLVOTextBubble::updateGeometry(LLDrawable *drawable)
{
 	if (!(gPipeline.hasRenderType(LLPipeline::RENDER_TYPE_VOLUME)))
	{
		return TRUE;
	}
	
	if (mVolumeChanged)
	{
		LLVolumeParams volume_params = getVolume()->getParams();
		setVolume(volume_params, 0);

		LLPipeline::sCompiles++;

		LLFace* facep = drawable->getFace(0);
		if (facep)
		{
			drawable->setNumFaces(getVolume()->getNumFaces(), facep->getPool(),
								  getTEImage(0));
		}
	}

	for (S32 i = 0, count = drawable->getNumFaces(); i < count; i++)
	{
		LLFace* face = drawable->getFace(i);
		if (face)
		{
			face->setTEOffset(i);
			face->setTexture(LLViewerFetchedTexture::sSmokeImagep);
			face->setState(LLFace::FULLBRIGHT);
		}
	}

	mVolumeChanged = FALSE;

	mDrawable->movePartition();
	return TRUE;
}

void LLVOTextBubble::updateFaceSize(S32 idx)
{
	LLFace* face = mDrawable->getFace(idx);
	if (!face) return;
	
	if (idx == 0 || idx == 2)
	{
		face->setSize(0,0);
	}
	else
	{
		const LLVolumeFace& vol_face = getVolume()->getVolumeFace(idx);
		face->setSize(vol_face.mNumVertices, vol_face.mNumIndices);
	}
}

void LLVOTextBubble::getGeometry(S32 idx,
								 LLStrider<LLVector3>& verticesp,
								 LLStrider<LLVector3>& normalsp, 
								 LLStrider<LLVector2>& texcoordsp,
								 LLStrider<LLColor4U>& colorsp, 
								 LLStrider<U16>& indicesp) 
{
	if (idx == 0 || idx == 2 || !mDrawable->getFace(idx))
	{
		return;
	}

	const LLVolumeFace& face = getVolume()->getVolumeFace(idx);

	LLVector4a pos;
	pos.load3(getPositionAgent().mV);

	LLVector4a scale;
	scale.load3(getScale().mV);

	LLColor4U color = LLColor4U(getTE(idx)->getColor());
	U32 offset = mDrawable->getFace(idx)->getGeomIndex();

	LLVector4a* dst_pos = (LLVector4a*) verticesp.get();
	LLVector4a* src_pos = (LLVector4a*) face.mPositions;

	LLVector4a* dst_norm = (LLVector4a*) normalsp.get();
	LLVector4a* src_norm  = (LLVector4a*) face.mNormals;

	LLVector2* dst_tc = (LLVector2*) texcoordsp.get();
	LLVector2* src_tc = (LLVector2*) face.mTexCoords;

	LLVector4a::memcpyNonAliased16((F32*) dst_norm, (F32*) src_norm, face.mNumVertices * 4 * sizeof(F32));
	LLVector4a::memcpyNonAliased16((F32*) dst_tc, (F32*) src_tc, face.mNumVertices * 2 * sizeof(F32));

	for (U32 i = 0; i < face.mNumVertices; i++)
	{
		LLVector4a t;
		t.setMul(src_pos[i], scale);
		dst_pos[i].setAdd(t, pos);
		*colorsp++ = color;
	}
	
	for (U32 i = 0; i < face.mNumIndices; i++)
	{
		*indicesp++ = face.mIndices[i] + offset;
	}
}

U32 LLVOTextBubble::getPartitionType() const
{ 
	return LLViewerRegion::PARTITION_PARTICLE; 
}
