/*
 * Frame.h
 *
 *  Created on: 14.09.2015
 *      Author: alexey slovesnov
 */

#pragma once

#include "CheckNewVersion.h"
#include "common/WordsBase.h"
#include "common/consts.h"

using MenuMap = std::map<ENUM_MENU, GtkWidget *>;

class Frame : WordsBase {
  static const int COMBOLINE_MARGIN = 3;

  GtkWidget *m_widget;
  GtkWidget *m_menu;
  GtkWidget *m_text[TEXTVIEW_SIZE];
  GtkWidget *m_helperUp;
  GtkWidget *m_combo[COMBOBOX_SIZE];
  GtkWidget *m_entry[ENTRY_SIZE];
  GtkWidget *m_status;
  GtkWidget *m_statusMessage;
  GtkWidget *m_searchTagLabel;
  GtkWidget *m_button[BUTTON_SIZE];
  GtkWidget *m_currentDictionary;
  GtkWidget *m_check;
  GtkWidget *m_comboline;
  GtkWidget *m_radio;
  GtkWidget *m_charactersLabel;
  SafePangoFontDesc m_font[FONT_SIZE];

  MenuMap m_menuMap;
  std::vector<GtkAccelGroup *> m_accelGroup;
  bool m_lockSignals;
  std::jthread m_thread;
  CheckNewVersion m_newVersion;
  guint m_debounceTimerId = 0;
  ENUM_ENTRY m_currentEntry = ENTRY_SIZE;
  gint m_currentEntryPos;

public:
  gulong m_positionSignalId = 0;
  int m_separatorPosition;
  GtkWidget *m_panedWidget = nullptr;

private:
  ENUM_STATE m_state;
  int m_tagIndex = 0;
  int m_tags = 0;
  struct TagRange {
    GtkTextMark *start_mark;
    GtkTextMark *end_mark;
  };
  std::vector<TagRange> m_found_tags;
  void clearTagMarks();

  gint getComboIndex(ENUM_COMBOBOX e) const;
  void setComboIndex(ENUM_COMBOBOX e, gint v);

  void updateComboValue(ENUM_COMBOBOX e);

  void aboutDialog();
  void setHelperPanel();

  void createImageCombo(ENUM_COMBOBOX e);
  GtkWidget *createTextCombo(ENUM_COMBOBOX e, VString v, int active);
  GtkWidget *createTextCombo(ENUM_COMBOBOX e); // empty
  GtkWidget *createTextCombo(ENUM_COMBOBOX e, int from, int to, int active);
  GtkWidget *createTextCombo(ENUM_COMBOBOX e, ENUM_STRING from, ENUM_STRING to,
                             int active);

  template <typename T>
  void addComboLineToHelper(T &&id, int from, int to, int active,
                            ENUM_STRING eid, bool any = false) {
    std::string s_id;
    if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
      s_id = std::forward<T>(id);
    } else {
      s_id = string(std::forward<T>(id));
    }
    std::string s_eid = eid == STRING_SIZE ? "" : string(eid);
    addComboLineToHelper(from, to, active, s_id, this->string(TO), s_eid, any);
  }

  void addComboLineToHelper(int from, int to, int active, std::string s1,
                            std::string s2, std::string s3, bool any = false);
  void addComboToHelper(ENUM_STRING from, ENUM_STRING to, int active,
                        ENUM_COMBOBOX comboboxId = COMBOBOX_HELPER0);

  void updateTags(int n);

  GtkWidget *createLabel(std::string s) { return gtk_label_new(s.c_str()); }
  GtkWidget *createLabel(ENUM_STRING e) { return createLabel(string(e)); }

  inline GtkWidget *add(GtkWidget *box) { return box; }

  template <typename WidgetT, typename... Args>
  GtkWidget *add(GtkWidget *box, WidgetT &&widget, bool expand = true,
                 Args &&...rest) {
    if (sizeof...(Args)) {
      add(box, widget, expand);
      return add(box, std::forward<Args>(rest)...);
    }
    using CleanT = std::decay_t<WidgetT>;
    GtkWidget *w;
    if constexpr (std::is_convertible_v<CleanT, GtkWidget *>) {
      w = GTK_WIDGET(widget);
    } else {
      w = createLabel(std::forward<WidgetT>(widget));
    }
    gtk_box_pack_start(GTK_BOX(box), w, expand, expand, 0);
    return box;
  }

  template <typename... T>
  GtkWidget *createBox(GtkOrientation o, int margin, T &&...p) {
    static_assert(sizeof...(T) % 2 == 0);
    GtkWidget *w = gtk_box_new(o, margin);
    return add(w, std::forward<T>(p)...);
  }

public:
  Frame();

  void destroy();
  void endJob();
  void routine(ENUM_JOB_TYPE e = JOB_TYPE_FULL);
  bool isSignalsLocked() { return m_lockSignals; }
  void lockSignals() { m_lockSignals = true; }
  void unlockSignals() { m_lockSignals = false; }

  void clickMenu(ENUM_MENU menu);

  void updateDictionary();
  void loadAndUpdateCurrentLanguage();
  void comboChanged(ENUM_COMBOBOX e);
  void radioChanged(GtkWidget *w);
  void clickButton(GtkWidget *button);

  void setMenuLabel(ENUM_MENU e, std::string const &text);
  std::string getMenuLabel(ENUM_MENU e);

  void stopThreadAndNewRoutine(ENUM_JOB_TYPE e = JOB_TYPE_FULL);
  void stopThread();
  void startThread(ENUM_JOB_TYPE e);

  void updateTextView(ENUM_TEXTVIEW e, std::string const &s);

  void entryFocusChanged(bool in);
  void removeAccelerators();
  void addAccelerators();

  void refillCombo(ENUM_COMBOBOX e, ENUM_STRING first, int length);
  void newVersionMessage();

  void setDebounceTimer(ENUM_ENTRY e);
  void debounceTimeout(ENUM_ENTRY e);

  void setLabel(GtkWidget *w, ENUM_STRING e);
  void setLabel(GtkWidget *w, const std::string &s);

  virtual std::string getEntryString(ENUM_ENTRY e) const override;
  virtual std::string getTextViewString() const override;
  virtual bool getCheck() const override;

  void entryChanged(ENUM_ENTRY e);

  GtkTextBuffer *tvBuffer(ENUM_TEXTVIEW e = TEXTVIEW_MAIN) const;
  std::string getProgramVersionString() const;
  void updateStatus();
  void setSensitiveOrderFilter(bool b);
  void setPlaceholder(ENUM_ENTRY e, ENUM_STRING s);
  void updateButton(ENUM_BUTTON e);

  StartStopButtonState getStartStopState() const;
  GtkWidget *createTextView(ENUM_TEXTVIEW e);
  GtkWidget *createEntry(ENUM_ENTRY e);
  void updateCharactersLabel();
  ENUM_COMBOBOX getLastCombobox();
  bool selectFont(const std::string &s, ENUM_FONT e);
  std::string getCssFromPango(ENUM_FONT e);
  void updateFont(ENUM_FONT e);
  void resetSettings(bool update);
  void switchDictionary();
};
