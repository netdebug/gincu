#ifndef GRECTRENDER_H
#define GRECTRENDER_H

#include "gincu/gcolor.h"
#include "gincu/ggeometry.h"
#include "gincu/grenderanchor.h"

#include <memory>

namespace gincu {

class GTransform;
struct GRenderInfo;
class GRectRenderData;
class GImageData;

class GRectRender
{
public:
	GRectRender();
	~GRectRender();

	void draw(const GTransform & transform, const GRenderInfo * renderInfo);

	void setColor(const GColor color);
	GColor getColor() const;

	void setSize(const GSize & size);
	GSize getSize() const;

	const std::shared_ptr<GRectRenderData> & getData() const { return this->data; }

private:
	std::shared_ptr<GRectRenderData> data;
};


} //namespace gincu

#endif