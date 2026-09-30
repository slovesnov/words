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
#ifdef LANGUAGE_BUTTON
  GtkWidget *m_currentDictionary;
#endif
  GtkWidget *m_currentLanguage;
  GtkWidget *m_check;
  GtkWidget *m_comboline;
  GtkWidget *m_radio;
  GtkWidget *m_charactersLabel;
  SafePangoFontDesc m_font[FONT_SIZE];

  std::chrono::steady_clock::time_point last_click_time[BUTTON_SIZE]{};

  MenuMap m_menuMap;
  std::vector<GtkAccelGroup *> m_accelGroup;
  bool m_lockSignals;
  std::jthread m_thread, m_managerThread;
  CheckNewVersion m_newVersion;
  guint m_debounceTimerId = 0;
  ENUM_ENTRY m_currentEntry = ENTRY_SIZE;
  gint m_currentEntryPos;

public:
  gulong m_positionSignalId = 0;
  int m_separatorPosition;
  GtkWidget *m_panedWidget = nullptr;
  bool m_maximized;
  int m_x, m_y, m_width, m_height;

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

  void aboutDialog();
  void setHelperPanel(bool ignoreStateBegin);

  gint getComboIndex(ENUM_COMBOBOX e) const;
  void setComboIndex(ENUM_COMBOBOX e, gint v);
  void updateComboValue(ENUM_COMBOBOX e);
  void createImageCombo(ENUM_COMBOBOX e);
  void refillCombo(ENUM_COMBOBOX e, const VString &v, int active);
  template <typename T>
  void refillCombo(ENUM_COMBOBOX e, T from, T to, int active = -1) {
    refillCombo(e, fromTo(from, to), active);
  }
  template <typename T> VString fromTo(T from, T to) {
    VString v;
    std::string s;
    for (int i = int(from); i <= int(to); i++) {
      if constexpr (std::is_integral_v<T>) {
        s = std::to_string(i);
      } else if constexpr (std::is_enum_v<T>) {
        s = string(i);
      }
      v.push_back(s);
    }
    return v;
  }
  GtkWidget *createTextCombo(ENUM_COMBOBOX e, VString v = {}, int active = -1);
  template <typename T>
  GtkWidget *createTextCombo(ENUM_COMBOBOX e, T from, T to, int active) {
    return createTextCombo(e, fromTo(from, to), active);
  }

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
  bool isSignalsLocked() { return m_lockSignals; }
  void lockSignals() { m_lockSignals = true; }
  void unlockSignals() { m_lockSignals = false; }

  void clickMenu(ENUM_MENU menu);

  void updateDictionary(bool change = false);
  void updateLanguage(bool change = false);
  void comboChanged(ENUM_COMBOBOX e);
  void radioChanged(GtkWidget *w);
  void clickButton(GtkWidget *button);

  void setMenuLabel(ENUM_MENU e, std::string const &text);
  std::string getMenuLabel(ENUM_MENU e);

  void updateTextView(ENUM_TEXTVIEW e, std::string const &s);

  void entryFocusChanged(bool in);
  void removeAccelerators();
  void addAccelerators();

  void newVersionMessage();

  void setDebounceTimer(ENUM_ENTRY e);
  void debounceTimeout(ENUM_ENTRY e);

  void setLabel(GtkWidget *w, ENUM_STRING e);
  void setLabel(GtkWidget *w, const std::string &s);

  std::string getTextViewString(ENUM_TEXTVIEW e, bool locale) const;

  virtual std::string getEntryString(ENUM_ENTRY e) const override;
  virtual bool getCheck() const override;

  void entryChanged(ENUM_ENTRY e);

  GtkTextBuffer *tvBuffer(ENUM_TEXTVIEW e = TEXTVIEW_MAIN) const;
  std::string getProgramVersionString() const;
  void updateStatus(ENUM_STATE state);
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
  void saveText();

  void job(ENUM_JOB_TYPE e = JOB_TYPE_FULL);

  template <typename PredicateOrBool, typename... Args>
  void updateSensitivity(PredicateOrBool &&target, Args... args) {
    int index = 0;

    auto should_enable = [&target](int j) -> bool {
      if constexpr (std::is_same_v<std::decay_t<PredicateOrBool>, bool>) {
        return target;
      } else {
        return target(j);
      }
    };

    (
        [this, &index, &should_enable](auto id) {
          using T = decltype(id);
          GtkWidget *widget = nullptr;

          if constexpr (std::is_same_v<T, ENUM_MENU>)
            widget = m_menuMap[id];
          else if constexpr (std::is_same_v<T, ENUM_COMBOBOX>)
            widget = m_combo[id];
          else if constexpr (std::is_same_v<T, ENUM_ENTRY>)
            widget = m_entry[id];
          else if constexpr (std::is_same_v<T, ENUM_BUTTON>)
            widget = m_button[id];
          else {
            // need c++23
            static_assert(false,
                          "Unsupported widget type passed to the lambda!");
          }
          if (widget) {
            gtk_widget_set_sensitive(widget, should_enable(index));
          }
          index++;
        }(args),
        ...);
  }

  void windowDeleteEvent();
  void textviewChanged(ENUM_TEXTVIEW e);
  void addHelp(ENUM_STRING e);
};
