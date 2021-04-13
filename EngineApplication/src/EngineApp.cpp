#include "Engine.h"

#include "math.h"
#include <sstream>

Graphics::UIElement::GraphicsUIElement animationProc(Graphics::UIElement::AnimationEvent animationEvent, Graphics::UIElement::GraphicsUIElement previousState, float elapsedSeconds)
{
	switch (animationEvent)
	{
	case Graphics::UIElement::AnimationEvent::Start:
	{
		break;
	}
	case Graphics::UIElement::AnimationEvent::Idle:
	{
		previousState.xScale = 100;
		previousState.yScale = 50;
		previousState.r = 1;
		previousState.g = 1;
		previousState.b = 1;
		break;
	}
	case Graphics::UIElement::AnimationEvent::Hover:
	{
		previousState.xScale = 100;
		previousState.yScale = 50;
		previousState.r = 0.8;
		previousState.g = 0.8;
		previousState.b = 0.8;
		break;
	}
	case Graphics::UIElement::AnimationEvent::Click:
	{
		previousState.xScale = 110;
		previousState.yScale = 60;
		previousState.r = 0.8;
		previousState.g = 0.8;
		previousState.b = 0.8;
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
		GetUIManager()->AddUIElement(Graphics::UIElement(150, 100, 100, 50, 1, 1, 1, 10, animationProc, { 1, 10, 10, 0 }));
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