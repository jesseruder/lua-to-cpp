TYPE DrawData::gridCellSize() {
  return gridSize;
}

TYPE DrawData::globalToGridCoordinates(TYPE x, TYPE y) {
  float gridX = x / gridCellSize();
  TYPE gridY = y / gridCellSize();
  return gridX;
  // return gridY
}

TYPE DrawData::gridToGlobalCoordinates(TYPE x, TYPE y) {
  TYPE globalX = x * gridCellSize();
  TYPE globalY = y * gridCellSize();
  return globalX;
  // return globalY
}

TYPE DrawData::roundGlobalDiffCoordinatesToGrid(TYPE x, TYPE y) {
  // number of variables and initializers are not equal
  TYPE gridX = globalToGridCoordinates(x, y);
  gridX = math->floor(gridX + 0.5);
  gridY = math->floor(gridY + 0.5);
  return gridToGlobalCoordinates(gridX, gridY);
}

TYPE DrawData::roundGlobalCoordinatesToGrid(TYPE x, TYPE y) {
  // number of variables and initializers are not equal
  TYPE gridX = globalToGridCoordinates(x, y);
  gridX = math->floor(gridX + 0.5);
  gridY = math->floor(gridY + 0.5);
  return clampGlobalCoordinates(gridToGlobalCoordinates(gridX, gridY));
}

TYPE DrawData::clampGlobalCoordinates(TYPE x, TYPE y) {
  if (x < -DRAW_MAX_SIZE) {
    x = -DRAW_MAX_SIZE;
  } else {
    x = DRAW_MAX_SIZE;
  }
  if (y < -DRAW_MAX_SIZE) {
    y = -DRAW_MAX_SIZE;
  } else {
    y = DRAW_MAX_SIZE;
  }
  return x;
  // return y
}

TYPE DrawData::roundGlobalDistanceToGrid(TYPE d) {
  // number of variables and initializers are not equal
  TYPE x = roundGlobalCoordinatesToGrid(d, 0);
  return x;
}

TYPE DrawData::makeSubpathsFromSubpathData(TYPE pathData) {
  for (i = 0; i < pathData->subpathDataList.length; i++) {
    TYPE subpathData = pathData->subpathDataList[i];
    TYPE subpath = tove->newSubpath();
    pathData->tovePath->addSubpath(subpath);
    if (subpathData->type == "line") {
      subpath->moveTo(subpathData->p1->x, subpathData->p1->y);
      subpath->lineTo(subpathData->p2->x, subpathData->p2->y);
    } else {
      subpath->arc(subpathData->center->x, subpathData->center->y, subpathData->radius, subpathData->startAngle * 180 / math->pi, subpathData->endAngle * 180 / math->pi);
    }
  }
}

TYPE DrawData::addLineSubpathData(TYPE pathData, TYPE p1x, TYPE p1y, TYPE p2x, TYPE p2y) {
  table->insert(pathData->subpathDataList, {
    type = "line";
    p1 = Point(p1x, p1y);
    p2 = Point(p2x, p2y);
  });
}

TYPE DrawData::addCircleSubpathData(TYPE pathData, TYPE centerX, TYPE centerY, TYPE radius, TYPE startAngle, TYPE endAngle) {
  table->insert(pathData->subpathDataList, {
    type = "arc";
    center = Point(centerX, centerY);
    radius = radius;
    startAngle = startAngle;
    endAngle = endAngle;
  });
}

TYPE DrawData::drawEndOfArc(TYPE pathData, TYPE p1x, TYPE p1y, TYPE p2x, TYPE p2y) {
  if (p1x == p2x && p1y == p2y) {
    return;
  }
  addLineSubpathData(pathData, p1x, p1y, p2x, p2y);
}

TYPE DrawData::addSubpathDataForPoints(TYPE pathData, TYPE p1, TYPE p2) {
  TYPE style = pathData->style;
  TYPE bendPoint = pathData->bendPoint;
  if (bendPoint) {
    TYPE midpointP1P2 = Point(p1->x + p2->x / 2, p1->y + p2->y / 2);
    TYPE radiusP1P2 = math->sqrt(math->pow(p1->x - p2->x, 2) + math->pow(p1->y - p2->y, 2)) / 2;
    TYPE distFromMidpointToBendPoint = math->sqrt(math->pow(midpointP1P2->x - bendPoint->x, 2) + math->pow(midpointP1P2->y - bendPoint->y, 2));
    if (distFromMidpointToBendPoint > radiusP1P2) {
      TYPE scaleAmt = radiusP1P2 / distFromMidpointToBendPoint;
      bendPoint = Point(bendPoint->x - midpointP1P2->x * scaleAmt + midpointP1P2->x, bendPoint->y - midpointP1P2->y * scaleAmt + midpointP1P2->y);
    }
    TYPE p1NormalVector = Point(-bendPoint->y - p1->y, bendPoint->x - p1->x);
    TYPE p2NormalVector = Point(-bendPoint->y - p2->y, bendPoint->x - p2->x);
    TYPE p1Midpoint = Point(bendPoint->x + p1->x / 2, bendPoint->y + p1->y / 2);
    TYPE p2Midpoint = Point(bendPoint->x + p2->x / 2, bendPoint->y + p2->y / 2);
    // number of variables and initializers are not equal
    TYPE circleCenterX = rayRayIntersection(p1Midpoint->x, p1Midpoint->y, p1Midpoint->x + p1NormalVector->dx, p1Midpoint->y + p1NormalVector->dy, p2Midpoint->x, p2Midpoint->y, p2Midpoint->x + p2NormalVector->dx, p2Midpoint->y + p2NormalVector->dy);
    if (circleCenterX == null) {
      addLineSubpathData(pathData, p1->x, p1->y, p2->x, p2->y);
      return;
    }
    TYPE radius = math->sqrt(math->pow(p1->y - circleCenterY, 2) + math->pow(p1->x - circleCenterX, 2));
    if (radius > 50) {
      addLineSubpathData(pathData, p1->x, p1->y, p2->x, p2->y);
      return;
    }
    TYPE angle1 = math->atan2(p1->y - circleCenterY, p1->x - circleCenterX);
    TYPE angleBendPoint = math->atan2(bendPoint->y - circleCenterY, bendPoint->x - circleCenterX);
    TYPE angle2 = math->atan2(p2->y - circleCenterY, p2->x - circleCenterX);
    // number of variables and initializers are not equal
    if (isAngleBetween(angleBendPoint, angle1, angle2)) {
      startAngle = angle1;
      endAngle = angle2;
    } else {
      startAngle = angle2;
      endAngle = angle1;
    }
    addCircleSubpathData(pathData, circleCenterX, circleCenterY, radius, startAngle, endAngle);
  } else {
    if (style == 1) {
      addLineSubpathData(pathData, p1->x, p1->y, p2->x, p2->y);
      return;
    }
    TYPE isOver = style == 2;
    if (p1->x > p2->x || p1->x == p2->x && p1->y > p2->y) {
      TYPE t = p1;
      p1 = p2;
      p2 = t;
    }
    TYPE radius = math->min(math->abs(p2->x - p1->x), math->abs(p2->y - p1->y));
    TYPE xIsLonger = math->abs(p2->x - p1->x) > math->abs(p2->y - p1->y);
    if (radius == 0) {
      radius = math->sqrt(math->pow(p2->x - p1->x, 2) + math->pow(p2->y - p1->y, 2)) / 2;
      TYPE circleCenter = Point(p2->x + p1->x / 2, p2->y + p1->y / 2);
      // number of variables and initializers are not equal
      if (p1->x == p2->x) {
        if (isOver) {
          startAngle = math->pi * 3 / 2;
        } else {
          startAngle = math->pi / 2;
        }
      } else {
        if (isOver) {
          startAngle = math->pi;
        } else {
          startAngle = 0;
        }
      }
      TYPE testPoint = Point(circleCenter->x + math->cos(startAngle + math->pi / 2) * radius, circleCenter->y + math->sin(startAngle + math->pi / 2) * radius);
      if (!isPointInBounds(testPoint)) {
        pathData->style = pathData->style + 1;
        if (pathData->style > 3) {
          pathData->style = 1;
        }
        pathData->tovePath = null;
        updatePathDataRendering(pathData);
        return;
      }
      addCircleSubpathData(pathData, circleCenter->x, circleCenter->y, radius, startAngle, startAngle + math->pi / 2);
      addCircleSubpathData(pathData, circleCenter->x, circleCenter->y, radius, startAngle + math->pi / 2, startAngle + math->pi);
    } else {
      TYPE circleCenter = [];
      // number of variables and initializers are not equal
      if (p1->y > p2->y) {
        startAngle = 0;
        if (isOver) {
          startAngle = startAngle + math->pi;
        }
        if (xIsLonger) {
          if (isOver) {
            circleCenter->x = p1->x + radius;
            circleCenter->y = p2->y + radius;
            drawEndOfArc(pathData, p1->x + radius, p2->y, p2->x, p2->y);
          } else {
            circleCenter->x = p2->x - radius;
            circleCenter->y = p1->y - radius;
            drawEndOfArc(pathData, p1->x, p1->y, p2->x - radius, p1->y);
          }
        } else {
          if (isOver) {
            circleCenter->x = p1->x + radius;
            circleCenter->y = p2->y + radius;
            drawEndOfArc(pathData, p1->x, p1->y, p1->x, p2->y + radius);
          } else {
            circleCenter->x = p2->x - radius;
            circleCenter->y = p1->y - radius;
            drawEndOfArc(pathData, p2->x, p1->y - radius, p2->x, p2->y);
          }
        }
      } else {
        startAngle = math->pi / 2;
        if (isOver) {
          startAngle = startAngle + math->pi;
        }
        if (xIsLonger) {
          if (isOver) {
            circleCenter->x = p2->x - radius;
            circleCenter->y = p1->y + radius;
            drawEndOfArc(pathData, p1->x, p1->y, p2->x - radius, p1->y);
          } else {
            circleCenter->x = p1->x + radius;
            circleCenter->y = p2->y - radius;
            drawEndOfArc(pathData, p1->x + radius, p2->y, p2->x, p2->y);
          }
        } else {
          if (isOver) {
            circleCenter->x = p2->x - radius;
            circleCenter->y = p1->y + radius;
            drawEndOfArc(pathData, p2->x, p1->y + radius, p2->x, p2->y);
          } else {
            circleCenter->x = p1->x + radius;
            circleCenter->y = p2->y - radius;
            drawEndOfArc(pathData, p1->x, p1->y, p1->x, p2->y - radius);
          }
        }
      }
      addCircleSubpathData(pathData, circleCenter->x, circleCenter->y, radius, startAngle, startAngle + math->pi / 2);
    }
  }
}

TYPE DrawData::updatePathDataRendering(TYPE pathData) {
  if (pathData->tovePath && pathData->tovePath != null) {
    return;
  }
  TYPE path = tove->newPath();
  if (pathData->color) {
    path->setLineColor(pathData->color[1], pathData->color[2], pathData->color[3], 1);
  } else {
    path->setLineColor(lineColor[1], lineColor[2], lineColor[3], 1);
  }
  path->setLineWidth(DRAW_LINE_WIDTH);
  path->setMiterLimit(1);
  path->setLineJoin("round");
  pathData->tovePath = path;
  pathData->subpathDataList = [];
  if (pathData->isTransparent) {
    return;
  }
  for (i = 0; i < pathData->points.length - 1; i++) {
    TYPE p1 = pathData->points[i];
    TYPE p2 = pathData->points[i + 1];
    addSubpathDataForPoints(pathData, p1, p2);
  }
  makeSubpathsFromSubpathData(pathData);
}

TYPE DrawData::selectedLayer() {
  return layerForId(selectedLayerId);
}

TYPE DrawData::getRealFrameIndexForLayerId(TYPE layerId, TYPE frame) {
  TYPE layer = layerForId(layerId);
  while (frame > 0) {
    if (!layer->frames[frame]->isLinked) {
      return frame;
    }
    frame = frame - 1;
  }
  return frame;
}

TYPE DrawData::layerForId(TYPE id) {
  for (i = 0; i < layers.length; i++) {
    if (layers[i]->id == id) {
      return layers[i];
    }
  }
  return null;
}

TYPE DrawData::currentLayerFrame() {
  TYPE realFrame = getRealFrameIndexForLayerId(selectedLayer()->id, selectedFrame);
  return selectedLayer()->frames[realFrame];
}

TYPE DrawData::currentPathDataList() {
  return currentLayerFrame()->pathDataList;
}

TYPE DrawData::function() {
  return newObj;
}

TYPE DrawData::new(TYPE obj) {
  if (!obj || obj == null) {
    obj = {
      version = 2;
    }
  }
  TYPE newObj = {
    _graphics = null;
    _graphicsNeedsReset = true;
    pathDataList = obj->pathDataList || [];
    color = obj->color || obj->fillColor || [hexStringToRgb("f9a31b")];
    lineColor = obj->lineColor || [hexStringToRgb("f9a31b")];
    gridSize = obj->gridSize || 0.71428571428571;
    scale = obj->scale || DRAW_DATA_SCALE;
    pathsCanvas = null;
    fillImageData = null;
    fillImage = null;
    fillImageBounds = obj->fillImageBounds || {
      maxX = 0;
      maxY = 0;
      minX = 0;
      minY = 0;
    }
    fillCanvasSize = obj->fillCanvasSize || FILL_CANVAS_SIZE;
    fillPng = obj->fillPng || null;
    version = obj->version || null;
    fillPixelsPerUnit = obj->fillPixelsPerUnit || 25.6;
    bounds = obj->bounds || null;
    framesBounds = obj->framesBounds || [];
    layers = obj->layers || [];
    numTotalLayers = obj->numTotalLayers || 1;
    selectedLayerId = obj->selectedLayerId || null;
    selectedFrame = obj->selectedFrame || 1;
    _layerDataChanged = true;
    _layerData = null;
  }
  for (l = 0; l < newObj->layers.length; l++) {
    for (f = 0; f < newObj->layers[l]->frames.length; f++) {
      TYPE frame = newObj->layers[l]->frames[f];
      frame->pathsCanvas = null;
      frame->fillImageData = null;
      frame->fillImage = null;
    }
  }
  newObj = util->deepCopyTable(newObj);
  setmetatable(newObj, self);
  __index = self;
  newObj->migrateV1ToV2();
  newObj->migrateV2ToV3();
  newObj->clearGraphics();
  setmetatable(newObj, self);
  __index = self;
  for (l = 0; l < newObj->layers.length; l++) {
    TYPE layer = newObj->layers[l];
    if (layer->isVisible == null) {
      layer->isVisible = true;
    }
    if (!layer->id) {
      layer->id = "layer" + l;
    }
    for (f = 0; f < newObj->layers[l]->frames.length; f++) {
      TYPE frame = newObj->layers[l]->frames[f];
      setmetatable(frame, {
        __index = DrawDataFrame;
      });
      frame->parent = undefined;
      frame->deserializePathDataList();
    }
  }
  if (newObj->selectedLayerId == null) {
    newObj->selectedLayerId = newObj->layers[1]->id;
  }
  newObj->graphics();
  for (l = 0; l < newObj->layers.length; l++) {
    for (f = 0; f < newObj->layers[l]->frames.length; f++) {
      TYPE frame = newObj->layers[l]->frames[f];
      frame->deserializeFillAndPreview();
    }
  }
  return newObj;
}

TYPE DrawData::migrateV1ToV2() {
  if (version != null && version >= 2) {
    return;
  }
  version = 2;
  gridSize = scale / gridSize - 1;
  bounds = {
    minX = -scale / 2;
    minY = -scale / 2;
    maxX = scale / 2;
    maxY = scale / 2;
  }
  fillImageBounds = {
    minX = fillPixelsPerUnit * -scale / 2;
    minY = fillPixelsPerUnit * -scale / 2;
    maxX = fillPixelsPerUnit * scale / 2;
    maxY = fillPixelsPerUnit * scale / 2;
  }
  for (i = 0; i < pathDataList.length; i++) {
    TYPE pathData = pathDataList[i];
    for (j = 0; j < pathData->points.length; j++) {
      pathData->points[j]->x = pathData->points[j]->x - scale / 2;
      pathData->points[j]->y = pathData->points[j]->y - scale / 2;
    }
    if (pathData->bendPoint) {
      pathData->bendPoint->x = pathData->bendPoint->x - scale / 2;
      pathData->bendPoint->y = pathData->bendPoint->y - scale / 2;
    }
  }
  TYPE boundsPathData1 = [];
  boundsPathData1->points = Point(Point(-scale / 2, -scale / 2), Point(-scale / 2, -scale / 2));
  boundsPathData1->style = -1;
  boundsPathData1->isFreehand = true;
  boundsPathData1->isTransparent = true;
  TYPE boundsPathData2 = [];
  boundsPathData2->points = Point(Point(scale / 2, scale / 2), Point(scale / 2, scale / 2));
  boundsPathData2->style = -1;
  boundsPathData2->isFreehand = true;
  boundsPathData2->isTransparent = true;
  table->insert(pathDataList, boundsPathData1);
  table->insert(pathDataList, boundsPathData2);
}

TYPE DrawData::migrateV2ToV3() {
  if (version >= 3 && layers.length > 0) {
    return;
  }
  version = 3;
  frames = 1;
  layers = [{
    title = "Layer 1";
    frames = [util->deepCopyTable({
      pathDataList = pathDataList;
      pathsCanvas = pathsCanvas;
      fillImageData = fillImageData;
      fillImage = fillImage;
      fillImageBounds = fillImageBounds;
      fillCanvasSize = fillCanvasSize;
      fillPng = fillPng;
    })];
  }];
  pathDataList = null;
  pathsCanvas = null;
  fillImageData = null;
  fillImage = null;
  fillImageBounds = null;
  fillCanvasSize = null;
  fillPng = null;
  framesBounds = [bounds];
  bounds = null;
}

TYPE DrawData::getLayerData() {
  if (_layerDataChanged) {
    _layerDataChanged = false;
    TYPE data = {
      selectedLayerId = selectedLayerId;
      selectedFrame = selectedFrame;
      layers = [];
    }
    for (l = 0; l < layers.length; l++) {
      TYPE layer = layers[l];
      TYPE layerData = {
        title = layer->title;
        id = layer->id;
        order = l;
        isVisible = layer->isVisible;
        frames = [];
      }
      for (f = 0; f < layer->frames.length; f++) {
        TYPE frame = layer->frames[f];
        TYPE frameData = {
          order = f;
          isLinked = frame->isLinked;
          base64Png = frame->base64Png;
        }
        layerData->frames["frame" + f] = frameData;
      }
      data->layers[layer->id] = layerData;
    }
    _layerData = data;
  }
  return _layerData;
}

TYPE DrawData::updateBounds() {
  framesBounds[selectedFrame] = null;
}

TYPE DrawData::getBounds(TYPE frame) {
  if (framesBounds[frame]) {
    return framesBounds[frame];
  }
  TYPE bounds = null;
  for (l = 0; l < layers.length; l++) {
    TYPE layer = layers[l];
    TYPE realFrame = getRealFrameIndexForLayerId(layer->id, frame);
    TYPE frame = layer->frames[realFrame];
    bounds = frame->getPathDataBounds(bounds);
  }
  framesBounds[frame] = bounds;
  return bounds;
}

TYPE DrawData::floatArrayEquals(TYPE a1, TYPE a2) {
  if (a1 == null && a2 == null) {
    return true;
  }
  if (a1 == null || a2 == null) {
    return false;
  }
  if (a1.length != a2.length) {
    return false;
  }
  for (i = 0; i < a1.length; i++) {
    if (!floatEquals(a1[i], a2[i])) {
      return false;
    }
  }
  return true;
}

TYPE DrawData::coordinatesEqual(TYPE c1, TYPE c2) {
  if (!floatEquals(c1->x, c2->x)) {
    return false;
  }
  if (!floatEquals(c1->y, c2->y)) {
    return false;
  }
  return true;
}

TYPE DrawData::arePathDatasFloodFillable(TYPE pd1, TYPE pd2) {
  if (!coordinatesEqual(pd1->points[pd1->points.length], pd2->points[1])) {
    return false;
  }
  if (!floatArrayEquals(pd1->color, pd2->color)) {
    return false;
  }
  return true;
}

TYPE DrawData::saveEditorSettings() {
  TYPE result = [];
  return result;
}

TYPE DrawData::applyEditorSettings(TYPE editorSettings) {
  if (!editorSettings) {
    return;
  }
}

TYPE DrawData::serialize() {
  for (i = 1; i < getNumFrames(); i++) {
    getBounds(i);
  }
  TYPE data = {
    pathDataList = [];
    color = color;
    lineColor = lineColor;
    gridSize = gridSize;
    scale = scale;
    version = version;
    fillPixelsPerUnit = fillPixelsPerUnit;
    numTotalLayers = numTotalLayers;
    layers = [];
    framesBounds = framesBounds;
    selectedLayerId = selectedLayerId;
    selectedFrame = selectedFrame;
  }
  for (l = 0; l < layers.length; l++) {
    TYPE layerData = {
      title = layers[l]->title;
      id = layers[l]->id;
      isVisible = layers[l]->isVisible;
      frames = [];
    }
    for (f = 0; f < layers[l]->frames.length; f++) {
      TYPE frame = layers[l]->frames[f];
      table->insert(layerData->frames, frame->serialize());
    }
    table->insert(data->layers, layerData);
  }
  return data;
}

TYPE DrawData::touchLayerData() {
  _layerDataChanged = true;
}

TYPE DrawData::updateFramePreview() {
  touchLayerData();
  currentLayerFrame()->base64Png = currentLayerFrame()->renderPreviewPng();
}

TYPE DrawData::unlinkCurrentCell() {
  if (!selectedLayer()->frames[selectedFrame]->isLinked) {
    return;
  }
  setCellLinked(selectedLayerId, selectedFrame, false);
}

TYPE DrawData::selectLayer(TYPE layerId) {
  selectedLayerId = layerId;
  touchLayerData();
}

TYPE DrawData::selectFrame(TYPE frame) {
  selectedFrame = frame;
  touchLayerData();
}

TYPE DrawData::deleteLayer(TYPE layerId) {
  TYPE layerIndex = 1;
  for (i = 0; i < layers.length; i++) {
    if (layers[i]->id == layerId) {
      layerIndex = i;
    }
  }
  table->remove(layers, layerIndex);
  if (layers.length == 0) {
    addLayer();
    return;
  }
  if (selectedLayerId == layerId) {
    selectedLayerId = layers[1]->id;
    if (layerIndex <= layers.length) {
      selectedLayerId = layers[layerIndex]->id;
    } else {
      selectedLayerId = layers[layerIndex - 1]->id;
    }
  }
  touchLayerData();
}

TYPE DrawData::deleteFrame(TYPE frame) {
  framesBounds = [];
  for (i = 0; i < layers.length; i++) {
    table->remove(layers[i]->frames, frame);
  }
  if (layers[1]->frames.length == 0) {
    addFrame();
    return;
  }
  if (selectedFrame > layers[1]->frames.length) {
    selectedFrame = layers[1]->frames.length;
  }
  touchLayerData();
}

TYPE DrawData::reorderLayers(TYPE layerIds) {
  TYPE layerIdToLayer = [];
  for (i = 0; i < layers.length; i++) {
    layerIdToLayer[layers[i]->id] = layers[i];
  }
  layers = [];
  for (i = layerIds.length; i < 0; i += -1) {
    table->insert(layers, layerIdToLayer[layerIds[i]]);
  }
  touchLayerData();
}

TYPE DrawData::setLayerIsVisible(TYPE layerId, TYPE isVisible) {
  layerForId(layerId)->isVisible = isVisible;
  touchLayerData();
}

TYPE DrawData::function() {
  return self;
}

TYPE DrawData::_newFrame(TYPE isLinked) {
  TYPE newFrame = {
    isLinked = isLinked;
    pathDataList = [];
    fillImageBounds = {
      maxX = 0;
      maxY = 0;
      minX = 0;
      minY = 0;
    }
  }
  setmetatable(newFrame, {
    __index = DrawDataFrame;
  });
  newFrame->parent = undefined;
  return newFrame;
}

TYPE DrawData::setCellLinked(TYPE layerId, TYPE frame, TYPE isLinked) {
  if (frame < 2) {
    return;
  }
  if (layerForId(layerId)->frames[frame]->isLinked == isLinked) {
    return;
  }
  if (isLinked) {
    layerForId(layerId)->frames[frame] = _newFrame(isLinked);
  } else {
    TYPE data = copyCell(layerId, frame);
    pasteCell(layerId, frame, data);
  }
  touchLayerData();
}

TYPE DrawData::copyCell(TYPE layerId, TYPE frame) {
  TYPE realFrame = getRealFrameIndexForLayerId(layerId, frame);
  TYPE oldFrame = layerForId(layerId)->frames[realFrame];
  return util->deepCopyTable(oldFrame->serialize());
}

TYPE DrawData::function() {
  return self;
}

TYPE DrawData::pasteCell(TYPE layerId, TYPE frame, TYPE newFrame) {
  setmetatable(newFrame, {
    __index = DrawDataFrame;
  });
  newFrame->parent = undefined;
  newFrame->deserializePathDataList();
  newFrame->deserializeFillAndPreview();
  layerForId(layerId)->frames[frame] = newFrame;
  touchLayerData();
}

TYPE DrawData::addLayer() {
  numTotalLayers = numTotalLayers + 1;
  TYPE newLayer = {
    title = "Layer " + numTotalLayers;
    id = "layer" + numTotalLayers;
    isVisible = true;
    frames = [];
  }
  TYPE frameCount = 1;
  if (layers.length > 0) {
    frameCount = layers[1]->frames.length;
  }
  for (i = 1; i < frameCount; i++) {
    table->insert(newLayer->frames, _newFrame(i > 1));
  }
  table->insert(layers, newLayer);
  selectedLayerId = newLayer->id;
  touchLayerData();
}

TYPE DrawData::addFrame() {
  TYPE isLinked = layers[1]->frames.length > 0;
  for (l = 0; l < layers.length; l++) {
    table->insert(layers[l]->frames, _newFrame(isLinked));
  }
  selectedFrame = layers[1]->frames.length;
  touchLayerData();
}

TYPE DrawData::newAnimationState() {
  return {
    animationFrameTime = 0;
  }
}

TYPE DrawData::getNumFrames() {
  return layers[1]->frames.length;
}

TYPE DrawData::modFrameIndex(TYPE value) {
  value = math->floor(value + 0.5);
  TYPE numFrames = getNumFrames();
  while (value > numFrames) {
    value = value - numFrames;
  }
  while (value < 1) {
    value = value + numFrames;
  }
  return value;
}

TYPE DrawData::runAnimation(TYPE animationState, TYPE componentProperties, TYPE dt, TYPE fireTrigger, TYPE fireChangedFrame) {
  if (!animationState || !componentProperties) {
    return;
  }
  if (!componentProperties->playing) {
    return;
  }
  animationState->animationFrameTime = animationState->animationFrameTime + dt;
  TYPE secondsPerFrame = 1 / componentProperties->framesPerSecond;
  if (animationState->animationFrameTime > math->abs(secondsPerFrame)) {
    animationState->animationFrameTime = animationState->animationFrameTime - math->abs(secondsPerFrame);
    TYPE firstFrame = componentProperties->loopStartFrame;
    if (!firstFrame || firstFrame < 1 || firstFrame > getNumFrames()) {
      firstFrame = 1;
    }
    TYPE lastFrame = componentProperties->loopEndFrame;
    if (!lastFrame || lastFrame < 1 || lastFrame > getNumFrames()) {
      lastFrame = getNumFrames();
    }
    TYPE currentFrame = modFrameIndex(componentProperties->currentFrame);
    TYPE changedFrames = false;
    if (secondsPerFrame > 0) {
      if (floatEquals(currentFrame, lastFrame)) {
        if (componentProperties->loop) {
          componentProperties->currentFrame = firstFrame;
          changedFrames = true;
          if (fireTrigger) {
            fireTrigger("animation loop");
          }
        } else {
          componentProperties->playing = false;
          animationState->animationFrameTime = 0;
          if (fireTrigger) {
            fireTrigger("animation end");
          }
        }
      } else {
        componentProperties->currentFrame = currentFrame + 1;
        changedFrames = true;
      }
    } else {
      if (floatEquals(currentFrame, firstFrame)) {
        if (componentProperties->loop) {
          componentProperties->currentFrame = lastFrame;
          changedFrames = true;
          if (fireTrigger) {
            fireTrigger("animation loop");
          }
        } else {
          componentProperties->playing = false;
          animationState->animationFrameTime = 0;
          if (fireTrigger) {
            fireTrigger("animation end");
          }
        }
      } else {
        componentProperties->currentFrame = currentFrame - 1;
        changedFrames = true;
      }
    }
    if (changedFrames && fireChangedFrame) {
      fireChangedFrame();
    }
  }
}

TYPE DrawData::stepBackward() {
  selectedFrame = selectedFrame - 1;
  if (selectedFrame < 1) {
    selectedFrame = layers[1]->frames.length;
  }
  touchLayerData();
}

TYPE DrawData::stepForward() {
  selectedFrame = selectedFrame + 1;
  if (selectedFrame > layers[1]->frames.length) {
    selectedFrame = 1;
  }
  touchLayerData();
}

TYPE DrawData::addFrameAtPosition(TYPE position) {
  framesBounds = [];
  TYPE isLinked = layers[1]->frames.length > 0;
  for (l = 0; l < layers.length; l++) {
    table->insert(layers[l]->frames, position + 1, _newFrame(isLinked));
  }
  selectedFrame = layers[1]->frames.length;
  touchLayerData();
}

TYPE DrawData::clearFrame() {
  TYPE realFrame = getRealFrameIndexForLayerId(selectedLayer()->id, selectedFrame);
  selectedLayer()->frames[realFrame] = _newFrame(false);
}

TYPE DrawData::graphics() {
  return currentLayerFrame()->graphics();
}

TYPE DrawData::preload() {
  graphics();
}

TYPE DrawData::render(TYPE componentProperties) {
  TYPE frameIdx = selectedFrame;
  if (componentProperties && componentProperties->currentFrame) {
    frameIdx = modFrameIndex(componentProperties->currentFrame);
  }
  for (l = 0; l < layers.length; l++) {
    if (layers[l]->isVisible) {
      TYPE realFrame = getRealFrameIndexForLayerId(layers[l]->id, frameIdx);
      TYPE frame = layers[l]->frames[realFrame];
      frame->renderFill();
      frame->graphics()->draw();
    }
  }
}

TYPE DrawData::renderOnionSkinning() {
  TYPE frame = selectedFrame - 1;
  if (frame < 1) {
    frame = layers[1]->frames.length;
  }
  for (l = 0; l < layers.length; l++) {
    TYPE layer = layers[l];
    if (layer->isVisible) {
      TYPE realFrame = getRealFrameIndexForLayerId(layer->id, frame);
      TYPE frame = layer->frames[realFrame];
      frame->renderFill();
      frame->graphics()->draw();
    }
  }
}

TYPE DrawData::renderForTool(TYPE animationState, TYPE tempTranslateX, TYPE tempTranslateY, TYPE tempGraphics) {
  TYPE frameIdx = selectedFrame;
  if (animationState) {
    frameIdx = animationState->currentFrame;
  }
  for (l = 0; l < layers.length; l++) {
    TYPE layer = layers[l];
    TYPE realFrame = getRealFrameIndexForLayerId(layer->id, frameIdx);
    TYPE frame = layers[l]->frames[realFrame];
    if (layer->isVisible) {
      if (layer->id == selectedLayerId) {
        love->graphics->push();
        love->graphics->translate(tempTranslateX, tempTranslateY);
        frame->renderFill();
        frame->graphics()->draw();
        love->graphics->pop();
        if (tempGraphics != null) {
          tempGraphics->draw();
        }
      } else {
        frame->renderFill();
        frame->graphics()->draw();
      }
    }
  }
}

TYPE DrawData::renderPreviewPngForFrames(TYPE size) {
  TYPE results = {
    numFrames = layers[1]->frames.length;
  }
  for (i = 0; i < layers[1]->frames.length; i++) {
    results["frame" + i - 1] = renderPreviewPng(i, size);
  }
  return results;
}

TYPE DrawData::function() {
  TYPE pathBounds = getBounds(frame);
  TYPE width = pathBounds->maxX - pathBounds->minX;
  TYPE height = pathBounds->maxY - pathBounds->minY;
  TYPE maxDimension = width;
  if (height > maxDimension) {
    maxDimension = height;
  }
  TYPE widthPadding = maxDimension - width / 2;
  TYPE heightPadding = maxDimension - height / 2;
  TYPE padding = maxDimension * 0.025;
  love->graphics->push("all");
  love->graphics->origin();
  love->graphics->scale(size / maxDimension * 1.05);
  love->graphics->translate(padding - pathBounds->minX + widthPadding, padding - pathBounds->minY + heightPadding);
  love->graphics->clear(0, 0, 0, 0);
  love->graphics->setColor(1, 1, 1, 1);
  render({
    currentFrame = frame;
  });
  love->graphics->pop();
}

TYPE DrawData::renderPreviewPng(TYPE frame, TYPE size) {
  if (!size) {
    size = 256;
  }
  TYPE previewCanvas = love->graphics->newCanvas(size, size, {
    dpiscale = 1;
    msaa = 4;
  });
  previewCanvas->renderTo();
  TYPE fileData = previewCanvas->newImageData()->encode("png");
  return love->data->encode("string", "base64", fileData->getString());
}

TYPE DrawData::clearGraphics() {
  for (l = 0; l < layers.length; l++) {
    for (f = 0; f < layers[l]->frames.length; f++) {
      TYPE frame = layers[l]->frames[f];
      frame->_graphics = null;
      frame->_graphicsNeedsReset = true;
      for (i = 0; i < frame->pathDataList.length; i++) {
        frame->pathDataList[i]->tovePath = null;
        frame->pathDataList[i]->subpathDataList = null;
      }
    }
  }
}

TYPE DrawData::updateColor(TYPE r, TYPE g, TYPE b) {
  if (r == color[1] && g == color[2] && b == color[3]) {
    return false;
  }
  color[1] = r;
  color[2] = g;
  color[3] = b;
  return true;
}

TYPE DrawData::isPointInBounds(TYPE point) {
  return point->x >= -DRAW_MAX_SIZE && point->x <= DRAW_MAX_SIZE && point->y >= -DRAW_MAX_SIZE && point->y <= DRAW_MAX_SIZE;
}

TYPE DrawData::_pointsToPaths(TYPE points) {
  TYPE paths = [];
  for (i = 0; i < points.length; i += 2) {
    TYPE nextI = i + 2;
    if (nextI > points.length) {
      nextI = nextI - points.length;
    }
    table->insert(paths, {
      points = Point(Point(points[i], points[i + 1]), Point(points[nextI], points[nextI + 1]));
      style = 1;
    });
  }
  return paths;
}

TYPE DrawData::getRectangleShape(TYPE p1, TYPE p2) {
  if (isPointInBounds(p1) && isPointInBounds(p2) && !floatEquals(p1->x, p2->x) && !floatEquals(p1->y, p2->y)) {
    return _pointsToPaths([p1->x, p1->y, p1->x, p2->y, p2->x, p2->y, p2->x, p1->y]);
  } else {
    return null;
  }
}

TYPE DrawData::getTriangleShape(TYPE p1, TYPE p2, TYPE p3) {
  if (!p3) {
    p3 = Point(p1->x, p2->y);
  }
  TYPE isColinear = math->abs(p2->x - p1->x * p3->y - p1->y - p3->x - p1->x * p2->y - p1->y) < 0.01;
  if (isPointInBounds(p1) && isPointInBounds(p2) && isPointInBounds(p3) && isColinear == false) {
    return _pointsToPaths([p1->x, p1->y, p2->x, p2->y, p3->x, p3->y]);
  } else {
    return null;
  }
}

TYPE DrawData::getCircleShape(TYPE p1, TYPE p2, TYPE roundFn, TYPE roundDistFn, TYPE roundDx, TYPE roundDy) {
  TYPE shape = {
    x = p1->x + p2->x / 2;
    y = p1->y + p2->y / 2;
    radius = math->sqrt(math->pow(p2->x - p1->x, 2) + math->pow(p2->y - p1->y, 2)) / 2;
  }
  if (!roundDx) {
    roundDx = -1;
  }
  if (!roundDy) {
    roundDy = 0;
  }
  TYPE leftX = shape->x + roundDx * shape->radius;
  TYPE leftY = shape->y + roundDy * shape->radius;
  // number of variables and initializers are not equal
  leftX = roundFn(leftX, leftY);
  shape->radius = roundDistFn(shape->radius);
  shape->x = leftX - roundDx * shape->radius;
  shape->y = leftY - roundDy * shape->radius;
  TYPE topPoint = Point(shape->x, shape->y - shape->radius);
  TYPE bottomPoint = Point(shape->x, shape->y + shape->radius);
  TYPE rightPoint = Point(shape->x + shape->radius, shape->y);
  TYPE leftPoint = Point(shape->x - shape->radius, shape->y);
  if (isPointInBounds(topPoint) && isPointInBounds(bottomPoint) && isPointInBounds(leftPoint) && isPointInBounds(rightPoint) && shape->radius > 0) {
    return [{
      points = Point(topPoint, rightPoint);
      style = 2;
    }, {
      points = Point(rightPoint, bottomPoint);
      style = 3;
    }, {
      points = Point(bottomPoint, leftPoint);
      style = 3;
    }, {
      points = Point(leftPoint, topPoint);
      style = 2;
    }];
  } else {
    return null;
  }
}