#pragma once

#include "../Macros.h"

#include <vector>

#pragma warning(push)
#pragma warning(disable:4251) //Disable DLL-Interface Warning
namespace Graphics
{
	class ENGINE_DLL_DS UIElement
	{
	public:
		struct GraphicsUIElement
		{
			unsigned int xPosition;
			unsigned int yPosition;
			unsigned int xScale;
			unsigned int yScale;

			float roundness;

			float r;
			float g;
			float b;
		};
		enum AnimationEvent
		{
			Start,
			Idle,
			Hover,
			Click
		};
	public:
		UIElement(unsigned int xPosition, unsigned int yPosition, unsigned int xScale, unsigned int yScale, float rChannel, float gChannel, float bChannel, float roundness);
		UIElement(unsigned int xPosition, unsigned int yPosition, unsigned int xScale, unsigned int yScale, float rChannel, float gChannel, float bChannel, float roundness, GraphicsUIElement(*animationFunction)(AnimationEvent animationEvent, UIElement::GraphicsUIElement previousState, float elapsedSeconds), std::vector<float> animationLengths);
		~UIElement() = default;
		UIElement(const UIElement&) = default;
		UIElement& operator = (const UIElement&) = default;
	public:
		float IsTouching(int mouseXPosition, int mouseYPosition);
		bool IsAnimated() const;
		bool IsPlayingAnimation() const;
		AnimationEvent GetCurrentEvent() const;
		GraphicsUIElement GetElementData(bool *ioUpdateBuffer);
	public:
		void PlayAnimation(AnimationEvent animationEvent);
		void UpdateAnimation(float currentTime);
	private:
		void UpdateState();
	private:
		unsigned int xPosition;
		unsigned int yPosition;
		unsigned int xScale;
		unsigned int yScale;
		float roundness;
		float r;
		float g;
		float b;

		GraphicsUIElement currentState;
		bool bufferUpToDate = false;

		bool isAnimated;
		bool isPlaying = false;
		AnimationEvent lastEvent = AnimationEvent::Start;
		float animationStartTime;
		std::vector<float> animationLengths = { 0, 0, 0, 0 }; //Length of each animation in seconds
		GraphicsUIElement(*animationFunc)(AnimationEvent animationEvent, UIElement::GraphicsUIElement, float elapsedSeconds);
	};
}
#pragma warning(pop)