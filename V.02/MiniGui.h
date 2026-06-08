#pragma once

#include <Arduino.h>
#include <U8g2lib.h>

namespace MiniGui {

enum class Event {
  TURN_LEFT,
  TURN_RIGHT,
  PRESS_BUTTON,
  LONG_HOLD_BUTTON
};

class View;
class Group;
class Screen;
class ScreenManager;

typedef void (*PressCallback)(View* sender);
typedef bool (*EventReader)(Event& event);

enum class MessageBoxResult {
  NONE = 0,
  YES,
  NO,
  OK,
  CANCEL,
  CUSTOM
};

enum class MessageBoxButtonType {
  TEXT,
  ICON
};

struct MessageBoxButton {
  MessageBoxResult result;
  MessageBoxButtonType type;
  const char* text;
  const uint8_t* icon16x16;
};

class View {
protected:
  int16_t x;
  int16_t y;
  int16_t w;
  int16_t h;

  bool visible = true;
  bool selectable = false;
  bool selected = false;
  bool editing = false;

  Group* parent = nullptr;
  ScreenManager* screenManager = nullptr;
  PressCallback pressCallback = nullptr;

public:
  View(int16_t x, int16_t y, int16_t w, int16_t h, bool selectable = false);
  virtual ~View() {}

  virtual void draw(U8G2& u8g2) = 0;
  virtual bool handleEvent(Event event);

  void setParent(Group* parent);
  Group* getParent() const;

  virtual void setScreenManager(ScreenManager* manager);
  ScreenManager* getScreenManager() const;
  void invalidate();

  void setVisible(bool value);
  bool isVisible() const;

  void setSelectable(bool value);
  bool isSelectable() const;

  void setSelected(bool value);
  bool isSelected() const;

  void setEditing(bool value);
  bool isEditing() const;

  void setPressCallback(PressCallback callback);
  void notifyPress();

  int16_t getX() const;
  int16_t getY() const;
  int16_t getW() const;
  int16_t getH() const;
};

class Group : public View {
protected:
  View** children;
  uint8_t childCount;
  int8_t selectedIndex = -1;
  bool selectCyclicMode = true;

public:
  Group(
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h,
    View** children,
    uint8_t childCount,
    bool selectCyclicMode = true
  );

  void draw(U8G2& u8g2) override;
  bool handleEvent(Event event) override;

  View* getSelectedChild();
  int8_t getSelectedIndex() const;

  virtual bool selectFirst();
  virtual bool selectLast();
  virtual bool selectNext();
  virtual bool selectPrevious();

  bool setSelectedIndex(uint8_t index);

  void setSelectCyclicMode(bool value);
  bool getSelectCyclicMode() const;

  void setScreenManager(ScreenManager* manager) override;
};

class Screen : public Group {
private:
  const char* name;

public:
  Screen(
    const char* name,
    View** children,
    uint8_t childCount,
    bool selectCyclicMode = false
  );

  Screen(
    const char* name,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h,
    View** children,
    uint8_t childCount,
    bool selectCyclicMode = false
  );

  const char* getName() const;

  virtual void onEnter();
  virtual void onExit();
  virtual void onUpdate();
};

class ScreenManager {
private:
  U8G2* u8g2 = nullptr;
  Screen* activeScreen = nullptr;
  bool invalidated = true;

public:
  ScreenManager(U8G2& u8g2);

  void setScreen(Screen* screen);
  Screen* getActiveScreen() const;

  bool handleEvent(Event event);

  void draw();
  void update();

  void invalidate();
  bool isInvalidated() const;
};

class Label : public View {
private:
  const char* text;
  const uint8_t* font;

public:
  Label(int16_t x, int16_t y, const char* text, const uint8_t* font);

  void setText(const char* text);
  const char* getText() const;

  void setFont(const uint8_t* font);
  const uint8_t* getFont() const;

  void draw(U8G2& u8g2) override;
};

class Icon : public View {
private:
  const uint8_t* bitmap;

public:
  Icon(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t* bitmap, bool selectable = false);

  void setBitmap(const uint8_t* bitmap);
  void draw(U8G2& u8g2) override;
};

class CheckButton : public View {
private:
  bool checked;

public:
  CheckButton(int16_t x, int16_t y, bool checked = false);

  void draw(U8G2& u8g2) override;
  bool handleEvent(Event event) override;

  bool isChecked() const;
  void setChecked(bool value);
};

class Number : public View {
private:
  int value;
  int minValue;
  int maxValue;
  const uint8_t* font;

public:
  Number(
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h,
    int value,
    int minValue,
    int maxValue,
    const uint8_t* font
  );

  void draw(U8G2& u8g2) override;
  bool handleEvent(Event event) override;

  int getValue() const;
  void setValue(int value);

  void setFont(const uint8_t* font);
  const uint8_t* getFont() const;
};

class List : public View {
private:
  const char** items;
  uint8_t itemCount;
  uint8_t visibleItemsCount;
  uint8_t selectedItemIndex = 0;
  uint8_t topItemIndex = 0;
  uint8_t itemHeight = 10;
  bool exitOnBoundary = false;

  const uint8_t* font;

public:
  List(
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h,
    const char** items,
    uint8_t itemCount,
    uint8_t visibleItemsCount,
    const uint8_t* font
  );

  void draw(U8G2& u8g2) override;
  bool handleEvent(Event event) override;

  uint8_t getSelectedItemIndex() const;
  const char* getSelectedItem() const;

  void setExitOnBoundary(bool value);
  bool getExitOnBoundary() const;

  void setFont(const uint8_t* font);
  const uint8_t* getFont() const;

private:
  bool moveUp();
  bool moveDown();
};


// Callback used by Pages widget.
// pageIndex is the currently displayed page number.
typedef void (*PageDrawCallback)(U8G2& u8g2, uint8_t pageIndex);

class Pages : public View {
private:
  PageDrawCallback* pageCallbacks;
  uint8_t pageCount;
  uint8_t currentPage = 0;
  bool cyclic = false;

public:
  Pages(
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h,
    PageDrawCallback* pageCallbacks,
    uint8_t pageCount,
    bool cyclic = false,
    bool selectable = true
  );

  void draw(U8G2& u8g2) override;
  bool handleEvent(Event event) override;

  uint8_t getCurrentPage() const;
  uint8_t getPageCount() const;

  bool nextPage();
  bool previousPage();

  void setCurrentPage(uint8_t page);
  void setCyclic(bool value);
  bool getCyclic() const;

private:
  bool hasPreviousPage() const;
  bool hasNextPage() const;
  void drawNavigationArrows(U8G2& u8g2);
};

class MessageBox : public Group {
private:
  const char* text;
  const uint8_t* font;
  MessageBoxButton* buttons;
  uint8_t buttonCount;
  MessageBoxResult result = MessageBoxResult::NONE;

  char line1[24];
  char line2[24];

public:
  MessageBox(
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h,
    const char* text,
    MessageBoxButton* buttons,
    uint8_t buttonCount,
    const uint8_t* font
  );

  void draw(U8G2& u8g2) override;
  bool handleEvent(Event event) override;

  MessageBoxResult getResult() const;

  bool selectFirst() override;
  bool selectLast() override;
  bool selectNext() override;
  bool selectPrevious() override;

private:
  void splitTextToTwoLines();
  void drawButton(U8G2& u8g2, uint8_t index, int16_t bx, int16_t by);
};

MessageBoxResult messageBox(
  U8G2& u8g2,
  const char* text,
  MessageBoxButton* buttons,
  uint8_t buttonCount,
  EventReader eventReader,
  const uint8_t* font
);

} // namespace MiniGui
