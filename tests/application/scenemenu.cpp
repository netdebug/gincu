#include "scenemenu.h"
#include "scenelogo.h"
#include "matchthree/scenematchthree.h"
#include "constants.h"
#include "gincu/gapplication.h"
#include "gincu/gscenemanager.h"
#include "gincu/ecs/gentity.h"
#include "gincu/ecs/gcomponentrender.h"
#include "gincu/ecs/gcomponenttransform.h"
#include "gincu/ecs/gcomponentlocaltransform.h"
#include "gincu/ecs/gcomponenttouchhandler.h"
#include "gincu/ecs/gcomponentanchor.h"
#include "gincu/ecs/gcomponentcamera.h"
#include "gincu/grenderanchor.h"
#include "gincu/gevent.h"

#include <algorithm>

namespace gincu {

void SceneMenu::returnToMainMenu()
{
	GApplication::getInstance()->getSceneManager()->switchScene(new SceneMenu());
}

void SceneMenu::doOnEnter()
{
	const GCoord viewWidth = this->getPrimaryCamera()->getWorldSize().width;
	const GCoord viewHeight = this->getPrimaryCamera()->getWorldSize().height;
	const GSize tileSize { 400, 80};
	const GCoord ySpace = 40;

	this->addEntity(
		(new GEntity())
		->addComponent(createComponent<GComponentTransform>(GPoint { -700, -230 }, GScale { 2.2f, 2.2f }))
			->addComponent(createAndLoadImageComponent(menuBackgroundImageName))
	);

	auto itemList = MenuRegister::getInstance()->getSortedItemList();
	const int itemCount = (int)itemList.size();
	const GCoord totalHeight = itemCount * tileSize.height + (itemCount - 1) * ySpace;
	const GCoord yStart = (viewHeight - totalHeight) / 2;
	const GCoord x = viewWidth / 2;

	for(int i = 0; i < itemCount; ++i) {
		const auto item = itemList[i];
		this->addEntity(
			(new GEntity())
				->addComponent(createComponent<GComponentTransform>())
				->addComponent(createComponent<GComponentLocalTransform>(GPoint { x, yStart + (tileSize.height + ySpace) * i }))
				->addComponent(createComponent<GComponentAnchor>(GRenderAnchor::center))
				->addComponent(createComponent<GComponentContainerRender>()
					->add(createRectRenderComponent(item.backgroundColor, tileSize))
					->add(createAndLoadTextComponent(item.caption, colorBlue, menuFontSize))
				)
				->addComponent(createComponent<GComponentRendererTouchHandler>()->addOnTouch([=](const GEvent & touchEvent) {
					if(touchEvent.getType() == GEventType::touchPressed) {
						item.callback();
					}
				}))
		);
	}
}

void SceneMenu::doOnExit()
{
}


MenuRegister * MenuRegister::getInstance()
{
	static MenuRegister instance;

	return &instance;
}

void MenuRegister::registerItem(const std::string & caption, const int order, const MenuCallback & callback, const GColor backgroundColor)
{
	this->itemList.push_back({
		caption,
		order,
		callback,
		backgroundColor
	});
}

const std::vector<MenuItem> & MenuRegister::getSortedItemList() const
{
	this->sortItemList();
	return this->itemList;
}

void MenuRegister::sortItemList() const
{
	std::sort(this->itemList.begin(), this->itemList.end(), [](const MenuItem & a, const MenuItem & b) { return a.order < b.order; });
}


} //namespace gincu
