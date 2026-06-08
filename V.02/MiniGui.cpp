#include "MiniGui.h"

namespace MiniGui {

// ================= View =================

View::View(int16_t x, int16_t y, int16_t w, int16_t h, bool selectable)
  : x(x), y(y), w(w), h(h), selectable(selectable) {}

bool View::handleEvent(Event event) {
  if (event == Event::PRESS_BUTTON && selectable) {
    notifyPress();
    return pressCallback != nullptr;
  }

  return false;
}

void View::setParent(Group* parent) {
  this->parent = parent;
}

Group* View::getParent() const {
  return parent;
}

void View::setScreenManager(ScreenManager* manager) {
  screenManager = manager;
}

ScreenManager* View::getScreenManager() const {
  return screenManager;
}

void View::invalidate() {
  if (screenManager != nullptr) {
    screenManager->invalidate();
  }
}

void View::setVisible(bool value) {
  visible = value;
}

bool View::isVisible() const {
  return visible;
}

void View::setSelectable(bool value) {
  selectable = value;
}

bool View::isSelectable() const {
  return selectable && visible;
}

void View::setSelected(bool value) {
  selected = value;
}

bool View::isSelected() const {
  return selected;
}

void View::setEditing(bool value) {
  editing = value;
}

bool View::isEditing() const {
  return editing;
}

void View::setPressCallback(PressCallback callback) {
  pressCallback = callback;
}

void View::notifyPress() {
  if (pressCallback != nullptr) {
    pressCallback(this);
  }
}

int16_t View::getX() const { return x; }
int16_t View::getY() const { return y; }
int16_t View::getW() const { return w; }
int16_t View::getH() const { return h; }

// ================= Group =================

Group::Group(
  int16_t x,
  int16_t y,
  int16_t w,
  int16_t h,
  View** children,
  uint8_t childCount,
  bool selectCyclicMode
)
  : View(x, y, w, h, false),
    children(children),
    childCount(childCount),
    selectCyclicMode(selectCyclicMode) {

  if (children != nullptr) {
    for (uint8_t i = 0; i < childCount; i++) {
      if (children[i] != nullptr) {
        children[i]->setParent(this);
      }
    }
  }

  selectFirst();
}

void Group::draw(U8G2& u8g2) {
  if (children == nullptr) return;

  for (uint8_t i = 0; i < childCount; i++) {
    if (children[i] != nullptr && children[i]->isVisible()) {
      children[i]->draw(u8g2);
    }
  }
}

bool Group::handleEvent(Event event) {
  View* selectedChild = getSelectedChild();

  if (selectedChild != nullptr && selectedChild->handleEvent(event)) {
    return true;
  }

  if (event == Event::TURN_LEFT) {
    return selectPrevious();
  }

  if (event == Event::TURN_RIGHT) {
    return selectNext();
  }

  return false;
}

View* Group::getSelectedChild() {
  if (children == nullptr || selectedIndex < 0 || selectedIndex >= childCount) {
    return nullptr;
  }

  return children[selectedIndex];
}

int8_t Group::getSelectedIndex() const {
  return selectedIndex;
}

bool Group::selectFirst() {
  if (children == nullptr || childCount == 0) {
    return false;
  }

  for (uint8_t i = 0; i < childCount; i++) {
    if (children[i] != nullptr && children[i]->isSelectable()) {
      return setSelectedIndex(i);
    }
  }

  return false;
}

bool Group::selectLast() {
  if (children == nullptr || childCount == 0) {
    return false;
  }

  for (int16_t i = childCount - 1; i >= 0; i--) {
    if (children[i] != nullptr && children[i]->isSelectable()) {
      return setSelectedIndex(i);
    }
  }

  return false;
}

bool Group::selectNext() {
  if (children == nullptr || childCount == 0) {
    return false;
  }

  if (selectedIndex < 0) {
    return selectFirst();
  }

  if (selectCyclicMode) {
    for (uint8_t step = 1; step <= childCount; step++) {
      uint8_t next = (selectedIndex + step) % childCount;

      if (children[next] != nullptr && children[next]->isSelectable()) {
        return setSelectedIndex(next);
      }
    }

    return false;
  }

  for (uint8_t i = selectedIndex + 1; i < childCount; i++) {
    if (children[i] != nullptr && children[i]->isSelectable()) {
      return setSelectedIndex(i);
    }
  }

  return false;
}

bool Group::selectPrevious() {
  if (children == nullptr || childCount == 0) {
    return false;
  }

  if (selectedIndex < 0) {
    return selectLast();
  }

  if (selectCyclicMode) {
    for (uint8_t step = 1; step <= childCount; step++) {
      uint8_t previous = (selectedIndex + childCount - step) % childCount;

      if (children[previous] != nullptr && children[previous]->isSelectable()) {
        return setSelectedIndex(previous);
      }
    }

    return false;
  }

  for (int16_t i = selectedIndex - 1; i >= 0; i--) {
    if (children[i] != nullptr && children[i]->isSelectable()) {
      return setSelectedIndex(i);
    }
  }

  return false;
}

bool Group::setSelectedIndex(uint8_t index) {
  if (children == nullptr || index >= childCount) {
    return false;
  }

  if (children[index] == nullptr || !children[index]->isSelectable()) {
    return false;
  }

  if (selectedIndex >= 0 && selectedIndex < childCount && children[selectedIndex] != nullptr) {
    children[selectedIndex]->setSelected(false);
  }

  selectedIndex = index;
  children[selectedIndex]->setSelected(true);
  return true;
}

void Group::setSelectCyclicMode(bool value) {
  selectCyclicMode = value;
}

bool Group::getSelectCyclicMode() const {
  return selectCyclicMode;
}

void Group::setScreenManager(ScreenManager* manager) {
  View::setScreenManager(manager);

  if (children == nullptr) {
    return;
  }

  for (uint8_t i = 0; i < childCount; i++) {
    if (children[i] != nullptr) {
      children[i]->setScreenManager(manager);
    }
  }
}

// ================= Screen =================

Screen::Screen(
  const char* name,
  View** children,
  uint8_t childCount,
  bool selectCyclicMode
)
  : Group(0, 0, 128, 64, children, childCount, selectCyclicMode),
    name(name) {}

Screen::Screen(
  const char* name,
  int16_t x,
  int16_t y,
  int16_t w,
  int16_t h,
  View** children,
  uint8_t childCount,
  bool selectCyclicMode
)
  : Group(x, y, w, h, children, childCount, selectCyclicMode),
    name(name) {}

const char* Screen::getName() const {
  return name;
}

void Screen::onEnter() {
}

void Screen::onExit() {
}

void Screen::onUpdate() {
}

// ================= ScreenManager =================

ScreenManager::ScreenManager(U8G2& u8g2)
  : u8g2(&u8g2) {}

void ScreenManager::setScreen(Screen* screen) {
  if (activeScreen == screen) {
    return;
  }

  if (activeScreen != nullptr) {
    activeScreen->onExit();
  }

  activeScreen = screen;

  if (activeScreen != nullptr) {
    activeScreen->setScreenManager(this);
    activeScreen->onEnter();
  }

  invalidate();
}

Screen* ScreenManager::getActiveScreen() const {
  return activeScreen;
}

bool ScreenManager::handleEvent(Event event) {
  if (activeScreen == nullptr) {
    return false;
  }

  bool handled = activeScreen->handleEvent(event);

  if (handled) {
    invalidate();
  }

  return handled;
}

void ScreenManager::draw() {
  if (u8g2 == nullptr || activeScreen == nullptr) {
    return;
  }

  u8g2->clearBuffer();
  activeScreen->draw(*u8g2);
  u8g2->sendBuffer();

  invalidated = false;
}

void ScreenManager::update() {
  if (activeScreen != nullptr) {
    activeScreen->onUpdate();
  }

  if (invalidated) {
    draw();
  }
}

void ScreenManager::invalidate() {
  invalidated = true;
}

bool ScreenManager::isInvalidated() const {
  return invalidated;
}

// ================= Label =================

Label::Label(int16_t x, int16_t y, const char* text, const uint8_t* font)
  : View(x, y, 0, 0, false), text(text), font(font) {}

void Label::setText(const char* text) {
  this->text = text;
  invalidate();
}

const char* Label::getText() const {
  return text;
}

void Label::setFont(const uint8_t* font) {
  this->font = font;
  invalidate();
}

const uint8_t* Label::getFont() const {
  return font;
}

void Label::draw(U8G2& u8g2) {
  if (font != nullptr) {
    u8g2.setFont(font);
  }

  if (text != nullptr) {
    u8g2.drawStr(x, y, text);
  }
}

// ================= Icon =================

Icon::Icon(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t* bitmap, bool selectable)
  : View(x, y, w, h, selectable), bitmap(bitmap) {}

void Icon::setBitmap(const uint8_t* bitmap) {
  this->bitmap = bitmap;
  invalidate();
}

void Icon::draw(U8G2& u8g2) {
  if (editing) {
    u8g2.drawBox(x - 2, y - 2, w + 4, h + 4);
    u8g2.setDrawColor(0);
    if (bitmap != nullptr) {
      u8g2.drawXBMP(x, y, w, h, bitmap);
    }
    u8g2.setDrawColor(1);
    return;
  }

  if (bitmap != nullptr) {
    u8g2.drawXBMP(x, y, w, h, bitmap);
  }

  if (selected) {
    u8g2.drawFrame(x - 2, y - 2, w + 4, h + 4);
  }
}

// ================= CheckButton =================

CheckButton::CheckButton(int16_t x, int16_t y, bool checked)
  : View(x, y, 10, 10, true), checked(checked) {}

void CheckButton::draw(U8G2& u8g2) {
  u8g2.drawFrame(x, y, 8, 8);

  if (checked) {
    u8g2.drawLine(x + 2, y + 4, x + 4, y + 6);
    u8g2.drawLine(x + 4, y + 6, x + 7, y + 1);
  }

  if (selected) {
    u8g2.drawFrame(x - 2, y - 2, 12, 12);
  }
}

bool CheckButton::handleEvent(Event event) {
  if (event == Event::PRESS_BUTTON) {
    checked = !checked;
    notifyPress();
    return true;
  }

  return false;
}

bool CheckButton::isChecked() const {
  return checked;
}

void CheckButton::setChecked(bool value) {
  checked = value;
  invalidate();
}

// ================= Number =================

Number::Number(
  int16_t x,
  int16_t y,
  int16_t w,
  int16_t h,
  int value,
  int minValue,
  int maxValue,
  const uint8_t* font
)
  : View(x, y, w, h, true),
    value(value),
    minValue(minValue),
    maxValue(maxValue),
    font(font) {}

void Number::draw(U8G2& u8g2) {
  if (font != nullptr) {
    u8g2.setFont(font);
  }

  char buffer[16];
  snprintf(buffer, sizeof(buffer), "%d", value);

  if (editing) {
    u8g2.drawBox(x, y, w, h);
    u8g2.setDrawColor(0);
    u8g2.drawStr(x + 3, y + h - 3, buffer);
    u8g2.setDrawColor(1);
    return;
  }

  u8g2.drawStr(x + 3, y + h - 3, buffer);

  if (selected) {
    u8g2.drawFrame(x, y, w, h);
  }
}

bool Number::handleEvent(Event event) {
  if (event == Event::PRESS_BUTTON) {
    editing = !editing;
    notifyPress();
    return true;
  }

  if (!editing) {
    return false;
  }

  if (event == Event::TURN_LEFT) {
    if (value > minValue) {
      value--;
    }
    return true;
  }

  if (event == Event::TURN_RIGHT) {
    if (value < maxValue) {
      value++;
    }
    return true;
  }

  if (event == Event::LONG_HOLD_BUTTON) {
    editing = false;
    return true;
  }

  return false;
}

int Number::getValue() const {
  return value;
}

void Number::setValue(int value) {
  if (value < minValue) value = minValue;
  if (value > maxValue) value = maxValue;
  this->value = value;
  invalidate();
}

void Number::setFont(const uint8_t* font) {
  this->font = font;
  invalidate();
}

const uint8_t* Number::getFont() const {
  return font;
}

// ================= List =================

List::List(
  int16_t x,
  int16_t y,
  int16_t w,
  int16_t h,
  const char** items,
  uint8_t itemCount,
  uint8_t visibleItemsCount,
  const uint8_t* font
)
  : View(x, y, w, h, true),
    items(items),
    itemCount(itemCount),
    visibleItemsCount(visibleItemsCount),
    font(font) {}

void List::draw(U8G2& u8g2) {
  if (font != nullptr) {
    u8g2.setFont(font);
  }

  for (uint8_t i = 0; i < visibleItemsCount; i++) {
    uint8_t itemIndex = topItemIndex + i;

    if (itemIndex >= itemCount) {
      break;
    }

    int16_t itemY = y + i * itemHeight;

    if (itemIndex == selectedItemIndex) {
      u8g2.drawBox(x, itemY, w, itemHeight);
      u8g2.setDrawColor(0);
      u8g2.drawStr(x + 2, itemY + 8, items[itemIndex]);
      u8g2.setDrawColor(1);
    } else {
      u8g2.drawStr(x + 2, itemY + 8, items[itemIndex]);
    }
  }

  if (selected) {
    u8g2.drawFrame(x - 1, y - 1, w + 2, h + 2);
  }
}

bool List::handleEvent(Event event) {
  if (event == Event::TURN_LEFT) {
    if (exitOnBoundary && selectedItemIndex == 0) {
      return false;
    }

    return moveUp();
  }

  if (event == Event::TURN_RIGHT) {
    if (exitOnBoundary && selectedItemIndex + 1 >= itemCount) {
      return false;
    }

    return moveDown();
  }

  if (event == Event::PRESS_BUTTON) {
    notifyPress();
    return true;
  }

  return false;
}

uint8_t List::getSelectedItemIndex() const {
  return selectedItemIndex;
}

const char* List::getSelectedItem() const {
  if (items == nullptr || selectedItemIndex >= itemCount) {
    return nullptr;
  }

  return items[selectedItemIndex];
}

void List::setFont(const uint8_t* font) {
  this->font = font;
  invalidate();
}

const uint8_t* List::getFont() const {
  return font;
}

bool List::moveUp() {
  if (selectedItemIndex > 0) {
    selectedItemIndex--;

    if (selectedItemIndex < topItemIndex) {
      topItemIndex = selectedItemIndex;
    }

    return true;
  }

  return false;
}

bool List::moveDown() {
  if (selectedItemIndex + 1 < itemCount) {
    selectedItemIndex++;

    if (selectedItemIndex >= topItemIndex + visibleItemsCount) {
      topItemIndex++;
    }

    return true;
  }

  return false;
}


void List::setExitOnBoundary(bool value) {
  exitOnBoundary = value;
  invalidate();
}

bool List::getExitOnBoundary() const {
  return exitOnBoundary;
}

// ================= Pages =================

Pages::Pages(
  int16_t x,
  int16_t y,
  int16_t w,
  int16_t h,
  PageDrawCallback* pageCallbacks,
  uint8_t pageCount,
  bool cyclic,
  bool selectable
)
  : View(x, y, w, h, selectable),
    pageCallbacks(pageCallbacks),
    pageCount(pageCount),
    currentPage(0),
    cyclic(cyclic) {}

void Pages::draw(U8G2& u8g2) {
  if (pageCallbacks != nullptr && currentPage < pageCount && pageCallbacks[currentPage] != nullptr) {
    pageCallbacks[currentPage](u8g2, currentPage);
  }

  drawNavigationArrows(u8g2);

  if (selected) {
    u8g2.drawFrame(x, y, w, h);
  }
}

bool Pages::handleEvent(Event event) {
  if (event == Event::TURN_LEFT) {
    return previousPage();
  }

  if (event == Event::TURN_RIGHT) {
    return nextPage();
  }

  if (event == Event::PRESS_BUTTON) {
    notifyPress();
    return true;
  }

  return false;
}

uint8_t Pages::getCurrentPage() const {
  return currentPage;
}

uint8_t Pages::getPageCount() const {
  return pageCount;
}

bool Pages::nextPage() {
  if (pageCount == 0) {
    return false;
  }

  if (currentPage + 1 < pageCount) {
    currentPage++;
    invalidate();
    return true;
  }

  if (cyclic) {
    currentPage = 0;
    invalidate();
    return true;
  }

  return false;
}

bool Pages::previousPage() {
  if (pageCount == 0) {
    return false;
  }

  if (currentPage > 0) {
    currentPage--;
    invalidate();
    return true;
  }

  if (cyclic) {
    currentPage = pageCount - 1;
    invalidate();
    return true;
  }

  return false;
}

void Pages::setCurrentPage(uint8_t page) {
  if (page < pageCount) {
    currentPage = page;
    invalidate();
  }
}

void Pages::setCyclic(bool value) {
  cyclic = value;
  invalidate();
}

bool Pages::getCyclic() const {
  return cyclic;
}

bool Pages::hasPreviousPage() const {
  if (pageCount == 0) {
    return false;
  }

  return cyclic || currentPage > 0;
}

bool Pages::hasNextPage() const {
  if (pageCount == 0) {
    return false;
  }

  return cyclic || currentPage + 1 < pageCount;
}

void Pages::drawNavigationArrows(U8G2& u8g2) {
  int16_t centerY = y + h / 2;

  if (hasPreviousPage()) {
    u8g2.drawTriangle(
      x + 2, centerY,
      x + 9, centerY - 5,
      x + 9, centerY + 5
    );
  }

  if (hasNextPage()) {
    u8g2.drawTriangle(
      x + w - 2, centerY,
      x + w - 9, centerY - 5,
      x + w - 9, centerY + 5
    );
  }
}

// ================= MessageBox =================

MessageBox::MessageBox(
  int16_t x,
  int16_t y,
  int16_t w,
  int16_t h,
  const char* text,
  MessageBoxButton* buttons,
  uint8_t buttonCount,
  const uint8_t* font
)
  : Group(x, y, w, h, nullptr, 0, true),
    text(text),
    font(font),
    buttons(buttons),
    buttonCount(buttonCount) {

  splitTextToTwoLines();
  selectFirst();
}

void MessageBox::draw(U8G2& u8g2) {
  if (font != nullptr) {
    u8g2.setFont(font);
  }

  u8g2.drawFrame(x, y, w, h);
  u8g2.drawFrame(x + 1, y + 1, w - 2, h - 2);

  u8g2.drawStr(x + 6, y + 14, line1);

  if (line2[0] != '\0') {
    u8g2.drawStr(x + 6, y + 26, line2);
  }

  const uint8_t buttonWidth = 30;
  const uint8_t buttonHeight = 18;
  const uint8_t spacing = 4;
  uint8_t totalWidth = buttonCount * buttonWidth + (buttonCount > 0 ? (buttonCount - 1) * spacing : 0);

  int16_t startX = x + (w - totalWidth) / 2;
  int16_t buttonY = y + h - 22;

  for (uint8_t i = 0; i < buttonCount; i++) {
    int16_t bx = startX + i * (buttonWidth + spacing);
    drawButton(u8g2, i, bx, buttonY);
  }

  u8g2.setDrawColor(1);
}

bool MessageBox::handleEvent(Event event) {
  if (event == Event::TURN_LEFT) {
    return selectPrevious();
  }

  if (event == Event::TURN_RIGHT) {
    return selectNext();
  }

  if (event == Event::PRESS_BUTTON) {
    if (selectedIndex >= 0 && selectedIndex < buttonCount) {
      result = buttons[selectedIndex].result;
      
      

      return true;
    }
  }

  if (event == Event::LONG_HOLD_BUTTON) {
    result = MessageBoxResult::CANCEL;
    return true;
  }

  return false;
}

MessageBoxResult MessageBox::getResult() const {
  return result;
}

bool MessageBox::selectFirst() {
  if (buttonCount == 0) return false;
  selectedIndex = 0;
  return true;
}

bool MessageBox::selectLast() {
  if (buttonCount == 0) return false;
  selectedIndex = buttonCount - 1;
  return true;
}

bool MessageBox::selectNext() {
  if (buttonCount == 0) return false;

  if (selectedIndex < 0) {
    return selectFirst();
  }

  if (selectedIndex + 1 < buttonCount) {
    selectedIndex++;
    return true;
  }

  if (selectCyclicMode) {
    selectedIndex = 0;
    return true;
  }

  return false;
}

bool MessageBox::selectPrevious() {
  if (buttonCount == 0) return false;

  if (selectedIndex < 0) {
    return selectLast();
  }

  if (selectedIndex > 0) {
    selectedIndex--;
    return true;
  }

  if (selectCyclicMode) {
    selectedIndex = buttonCount - 1;
    return true;
  }

  return false;
}

void MessageBox::splitTextToTwoLines() {
  line1[0] = '\0';
  line2[0] = '\0';

  if (text == nullptr) {
    return;
  }

  const uint8_t maxChars = 21;
  uint8_t len = strlen(text);

  if (len <= maxChars) {
    strncpy(line1, text, sizeof(line1) - 1);
    line1[sizeof(line1) - 1] = '\0';
    return;
  }

  int8_t splitIndex = -1;

  for (int8_t i = maxChars; i > 0; i--) {
    if (text[i] == ' ') {
      splitIndex = i;
      break;
    }
  }

  if (splitIndex < 0) {
    splitIndex = maxChars;
  }

  strncpy(line1, text, splitIndex);
  line1[splitIndex] = '\0';

  const char* secondPart = text + splitIndex;

  while (*secondPart == ' ') {
    secondPart++;
  }

  strncpy(line2, secondPart, sizeof(line2) - 1);
  line2[sizeof(line2) - 1] = '\0';
}

void MessageBox::drawButton(U8G2& u8g2, uint8_t index, int16_t bx, int16_t by) {
  const uint8_t buttonWidth = 30;
  const uint8_t buttonHeight = 18;
  bool isSelectedButton = (index == selectedIndex);

  if (isSelectedButton) {
    u8g2.drawBox(bx, by, buttonWidth, buttonHeight);
    u8g2.setDrawColor(0);
  } else {
    u8g2.drawFrame(bx, by, buttonWidth, buttonHeight);
  }

  if (buttons[index].type == MessageBoxButtonType::ICON) {
    if (buttons[index].icon16x16 != nullptr) {
      u8g2.drawXBMP(bx + 7, by + 1, 16, 16, buttons[index].icon16x16);
    }
  } else {
    const char* label = buttons[index].text;

    if (label != nullptr) {
      u8g2.drawStr(bx + 4, by + 12, label);
    }
  }

  u8g2.setDrawColor(1);

  if (isSelectedButton) {
    u8g2.drawFrame(bx, by, buttonWidth, buttonHeight);
  }
}

MessageBoxResult messageBox(
  U8G2& u8g2,
  const char* text,
  MessageBoxButton* buttons,
  uint8_t buttonCount,
  EventReader eventReader,
  const uint8_t* font
) {
  MessageBox box(
    8,
    10,
    112,
    44,
    text,
    buttons,
    buttonCount,
    font
  );

  box.setSelectCyclicMode(true);
  box.selectFirst();

  while (box.getResult() == MessageBoxResult::NONE) {
    Event event;

    if (eventReader != nullptr && eventReader(event)) {
      box.handleEvent(event);
    }

    u8g2.clearBuffer();
    box.draw(u8g2);
    u8g2.sendBuffer();
  }

  return box.getResult();
}

} // namespace MiniGui
