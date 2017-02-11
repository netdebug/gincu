#include "gincu/grenderengine.h"
#include "gincu/gtransform.h"
#include "gincu/gimage.h"
#include "gincu/gspritesheetrender.h"
#include "gincu/gtextrender.h"
#include "gincu/grectrender.h"
#include "gincu/gapplication.h"
#include "gincu/grenderinfo.h"
#include "gsfmlutil.h"
#include "gimagedata.h"
#include "gtextrenderdata.h"
#include "grectrenderdata.h"
#include "grenderenginedata.h"

#include <cassert>

namespace gincu {

GRenderEngineData::GRenderEngineData()
	:
		window(),
		view(),
		batchDrawVertexArray(sf::Triangles),
		batchDrawRenderInfo(),
		inBatchDraw(false)
{
}

void GRenderEngineData::clearBatchDrawState() {
	this->batchDrawVertexArray.clear();
	this->batchDrawImageData.reset();
	this->inBatchDraw = false;
}

void GRenderEngineData::directDrawTexture(const std::shared_ptr<GImageData> & texture, const GRect & rect, const GTransform & transform, const GRenderInfo * renderInfo)
{
	const sf::Transform & sfmlTransform = transform.getSfmlTransform();

	sf::Sprite sprite(texture->texture, { (int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height });
	sf::RenderStates renderStates(sfmlTransform);
	copyBlendAndShaderToSfml(&renderStates, renderInfo);
	this->window->draw(sprite, renderStates);
}

void GRenderEngineData::batchDrawTexture(const std::shared_ptr<GImageData> & texture, const GRect & rect, const GTransform & transform, const GRenderInfo * renderInfo)
{
	const sf::Transform & sfmlTransform = transform.getSfmlTransform();

	this->batchDrawRenderInfo = *renderInfo;
	this->batchDrawImageData = texture;

	sf::VertexArray & vertexArray = this->batchDrawVertexArray;
	std::size_t count = vertexArray.getVertexCount();
	vertexArray.resize(count + 6);

	vertexArray[count].position = sfmlTransform.transformPoint({ 0, 0 });
	vertexArray[count].texCoords = { rect.x, rect.y };
	++count;
	vertexArray[count].position = sfmlTransform.transformPoint({ rect.width, 0 });
	vertexArray[count].texCoords = { rect.x + rect.width, rect.y };
	++count;
	vertexArray[count].position = sfmlTransform.transformPoint({ rect.width, rect.height });
	vertexArray[count].texCoords = { rect.x + rect.width, rect.y + rect.height };
	++count;

	vertexArray[count].position = sfmlTransform.transformPoint({ rect.width, rect.height });
	vertexArray[count].texCoords = { rect.x + rect.width, rect.y + rect.height };
	++count;
	vertexArray[count].position = sfmlTransform.transformPoint({ 0, rect.height });
	vertexArray[count].texCoords = { rect.x, rect.y + rect.height };
	++count;
	vertexArray[count].position = sfmlTransform.transformPoint({ 0, 0 });
	vertexArray[count].texCoords = { rect.x, rect.y };
}

void GRenderEngineData::beginBatchDraw()
{
	this->clearBatchDrawState();
	this->inBatchDraw = true;
}

void GRenderEngineData::endBatchDraw()
{
	this->flush();

	if(this->inBatchDraw) {
		if(this->batchDrawImageData) {
			sf::RenderStates renderStates(&this->batchDrawImageData->texture);
			copyBlendAndShaderToSfml(&renderStates, &this->batchDrawRenderInfo);
			this->window->draw(this->batchDrawVertexArray, renderStates);
		}

		this->clearBatchDrawState();
	}
}

void GRenderEngineData::flush()
{
	GCachedRenderItem & cachedItem = this->cachedRenderItem;
	switch(cachedItem.type) {
	case GCachedRenderType::texture:
		this->directDrawTexture(cachedItem.imageData, cachedItem.imageRect, cachedItem.imageTransform, &cachedItem.imageRenderInfo);
		break;

	case GCachedRenderType::none:
		break;
	}

	cachedItem.reset();
}


void GRenderEngine::doInitialize()
{
}

void GRenderEngine::doFinalize()
{
}

void GRenderEngine::render()
{
	this->data->window->clear(gameColorToSfml(GApplication::getInstance()->getConfigInfo().backgroundColor));

	this->renderList();

	this->data->endBatchDraw();

	this->data->window->display();
}

void GRenderEngine::draw(const GTextRender & text, const GTransform & transform, const GRenderInfo * renderInfo)
{
	this->data->endBatchDraw();

	sf::RenderStates renderStates(transform.getSfmlTransform());
	copyBlendAndShaderToSfml(&renderStates, renderInfo);
	this->data->window->draw(text.getData()->text, renderStates);
}

void GRenderEngine::draw(const GRectRender & rect, const GTransform & transform, const GRenderInfo * renderInfo)
{
	this->data->endBatchDraw();

	sf::RenderStates renderStates(transform.getSfmlTransform());
	copyBlendAndShaderToSfml(&renderStates, renderInfo);
	this->data->window->draw(rect.getData()->rectangle, renderStates);
}

void GRenderEngine::doDrawTexture(const std::shared_ptr<GImageData> & texture, const GRect & rect, const GTransform & transform, const GRenderInfo * renderInfo)
{
	if(! texture) {
		return;
	}

	GCachedRenderItem & cachedItem = this->data->cachedRenderItem;
	if(cachedItem.type != GCachedRenderType::none && cachedItem.type != GCachedRenderType::texture) {
		this->data->flush();
	}

	bool needCache = false;
	if(this->data->inBatchDraw) {
		if(texture == this->data->batchDrawImageData
			&& *renderInfo == this->data->batchDrawRenderInfo) {
			this->data->batchDrawTexture(texture, rect, transform, renderInfo);
		}
		else {
			this->data->endBatchDraw();
			needCache = true;
		}
	}
	else {
		if(texture == cachedItem.imageData
			&& *renderInfo == cachedItem.imageRenderInfo) {
			this->data->beginBatchDraw();
			this->data->batchDrawTexture(cachedItem.imageData, cachedItem.imageRect, cachedItem.imageTransform, &cachedItem.imageRenderInfo);
			this->data->batchDrawTexture(texture, rect, transform, renderInfo);
		}
		else {
			if(cachedItem.imageData) {
				this->data->flush();
			}
			needCache = true;
		}
	}
	if(needCache) {
		cachedItem.type = GCachedRenderType::texture;
		cachedItem.imageData = texture;
		cachedItem.imageRect = rect;
		cachedItem.imageTransform = transform;
		cachedItem.imageRenderInfo = *renderInfo;
	}
}


} //namespace gincu