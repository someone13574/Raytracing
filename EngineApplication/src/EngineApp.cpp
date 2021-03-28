#include "Engine.h"

#include "math.h"
#include <sstream>

Graphics::UIElement::GraphicsUIElement animationProc(Graphics::UIElement::AnimationEvent animationEvent, Graphics::UIElement::GraphicsUIElement previousState, float elapsedSeconds)
{
	switch (animationEvent)
	{
	case Graphics::UIElement::AnimationEvent::Start:
	{
		previousState.roundness = elapsedSeconds * 20;
		break;
	}
	case Graphics::UIElement::AnimationEvent::Idle:
	{
		previousState.r = 1;
		previousState.g = 1;
		previousState.b = 1;
		previousState.roundness = 10;
		previousState.xScale = 800;
		previousState.yScale = 380;
		break;
	}
	case Graphics::UIElement::AnimationEvent::Hover:
	{
		previousState.r = 0.8;
		previousState.g = 0.8;
		previousState.b = 0.8;
		previousState.roundness = 10;
		previousState.xScale = 800;
		previousState.yScale = 380;
		break;
	}
	case Graphics::UIElement::AnimationEvent::Click:
	{
		previousState.r = 0.6;
		previousState.g = 0.6;
		previousState.b = 0.6;
		previousState.roundness = 10;
		previousState.xScale = 810;
		previousState.yScale = 390;
		break;
	}
	}

	return previousState;
}

class App : public Engine::Application
{
public:
	App()
	{
		GetUIManager()->AddUIElement(Graphics::UIElement(960, 540, 800, 380, 1, 1, 1, 10, animationProc, { 10, 10, 10, 0 }));
		GetUIManager()->AddUIElement(Graphics::UIElement(960, 100, 800, 50, 1, 0, 0, 10));
	}

	~App()
	{

	}

	void Update()
	{
		GetGraphics()->Render();
	}
};

Engine::Application* Engine::CreateApplication()
{
	return new App();
}