
#pragma once

#include <forward_list>

#include <OgreTexture.h>

#include "Renderer.h"
#include "MultiRTTSampler.h"
#include "Keyframe.h"
#include "Utils.h"

namespace Vulnus
{
	struct FramePosition
	{
		FramePosition( double position, Keyframe *kf )
			: pos(position), frame(kf) { }

		double pos;
		Keyframe *frame;

		inline bool operator< ( const FramePosition &rhs ){ return pos < rhs.pos; }
	};


	class WoundTimeline : public MultiRTTSampler
	{
		public:
			WoundTimeline();
			~WoundTimeline();

			void initialize( _In_ ID3D11Device *dxDev, _In_ Renderer *ren, _In_ const std::vector<Utils::HullPoint> *patchEdgeListPtr );
			bool isInitialized() const { return isInit; }


			void updateFrames();
			SampledTexture sample( double position );

		private:
			template<class T> static void safeRelease(T **ppT)
			{
				if (*ppT)
				{
					(*ppT)->Release();
					*ppT = NULL;
				}
			}

			void addKeyframe( double position, _In_ Keyframe *kf );

			bool isInit;

			ID3D11Device *dxDevice;
			ID3D11DeviceContext *dxDevContext;

			//TODO: this is a cheap solution for managing the keyframes => create a more elegant solution
			std::map<Keyframe*, int> uniqueFrames;

			std::list<FramePosition> keyFrames;
			std::forward_list<FramePosition> timelineFrames;
	};

	bool sortFrames( const FramePosition f1, const FramePosition f2 );
}
