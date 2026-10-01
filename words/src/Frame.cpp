/*
 * Frame.cpp
 *
 *  Created on: 14.09.2015
 *      Author: alexey slovesnov
 */

#include "Frame.h"
#include "CheckNewVersion.h"
#include <format>
#include <magic_enum.hpp>

const char markTag[] = "mark";
const char activeTag[] = "active";
const char CERROR[] = "cerror";
const int TIMER = 500; // milliseconds
const int TIMER_BUTTON = 1000;
const int MIN_LEFT_PANEL_WIDTH = 700; // 800
const int MIN_RIGHT_PANEL_WIDTH = 420;
const int DEFAULT_SEPARATOR_POSITION = 1340;
const int TEXT_VIEW_MARGIN = 5;
const std::string CONFIG_TAGS[] = {"version",   "language", "dictionary",
                                   "separator", "fontout",  "fontcontrols",
                                   "maximized", "x",        "y",
                                   "width",     "height"};

Frame *frame;

void menu_activate(GtkWidget *widget, ENUM_MENU menu) {
  frame->clickMenu(menu);
}

void combo_changed(GtkComboBox *comboBox, ENUM_COMBOBOX e) {
  if (!frame->isSignalsLocked()) {
    frame->comboChanged(e);
  }
}

void entry_insert(GtkWidget *entry, gchar *new_text, gint new_text_length,
                  gpointer position, ENUM_ENTRY e) {
  frame->entryChanged(e);
}

void entry_delete(GtkWidget *entry, gint start_pos, gint end_pos,
                  ENUM_ENTRY e) {
  frame->entryChanged(e);
}

gboolean entry_focus_in(GtkWidget *widget, GdkEvent *event, gpointer) {
  frame->entryFocusChanged(true);
  return TRUE;
}

gboolean entry_focus_out(GtkWidget *widget, GdkEvent *event, gpointer) {
  frame->entryFocusChanged(false);
  return TRUE;
}

void button_clicked(GtkWidget *button, gpointer) { frame->clickButton(button); }

gboolean label_clicked(GtkWidget *label, const gchar *uri, gpointer) {
  openURL(uri);
  return TRUE;
}

void check_changed(GtkWidget *check, gpointer) { frame->checkChanged(); }

void radio_changed(GtkWidget *radio, gpointer) {
  // on change radid, got 2 signals. One radio became active, another not active
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio))) {
    frame->radioChanged();
  }
}

void textview_changed(GtkTextBuffer *buffer, ENUM_TEXTVIEW e) {
  frame->textviewChanged(e);
}

void destroy_window(GtkWidget *object, gpointer) { frame->destroy(); }

gboolean on_window_state_event(GtkWidget *widget, GdkEventWindowState *event,
                               gpointer user_data) {
  if (event->changed_mask & GDK_WINDOW_STATE_MAXIMIZED) {
    frame->m_maximized =
        (event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED) != 0;
  }
  return FALSE;
}

gboolean on_window_delete_event(GtkWidget *widget, GdkEvent *event,
                                gpointer user_data) {
  frame->windowDeleteEvent();
  return FALSE;
}

gboolean on_debounce_timeout(gpointer data) {
  frame->debounceTimeout(ENUM_ENTRY(GPOINTER_TO_INT(data)));
  return G_SOURCE_REMOVE;
}

gboolean update_status(gpointer data) {
  frame->updateStatus(ENUM_STATE(GPOINTER_TO_INT(data)));
  return G_SOURCE_REMOVE;
}

gboolean end_job(gpointer data) {
  frame->endJob();
  return G_SOURCE_REMOVE;
}

gboolean new_version_message(gpointer) {
  frame->newVersionMessage();
  return G_SOURCE_REMOVE;
}

Frame::Frame() : WordsBase() {
  GtkWidget *w, *w1, *w2;
  GtkWidget *item;
  bool bSubMenu;
  int i;
  std::vector<GtkMenuItem *> subMenu;
  std::string s;
  std::array<PangoFontDescription *, FONT_SIZE> p;
  m_newVersion.start(WORDS_VERSION, new_version_message);
  frame = this;
  m_menuClick = MENU_SEARCH;
  lockSignals(); // lock, unlock after create

  resetSettings(false);
  if (readConfig(CONFIG_TAGS, s, m_languageIndex, m_dictionaryIndex,
                 m_separatorPosition, p[0], p[1], m_maximized, m_x, m_y,
                 m_width, m_height)) {
    i = -1;
    for (auto &a : p) {
      i++;
      if (a)
        m_font[i].reset(a);
    }
  }
  // update font later

  m_widget = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  m_menu = gtk_menu_bar_new();

  m_helperUp = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);

  m_status = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
  m_statusMessage = gtk_label_new("");

  createImageCombo(COMBOBOX_SORT_ORDER, 1);
  createTextCombo(COMBOBOX_SORT);
  createTextCombo(COMBOBOX_FILTER);

  for (auto a : {ENTRY_SEARCH, ENTRY_FILTER}) {
    createEntry(a);
  }

  m_searchTagLabel = gtk_label_new("");
  gtk_widget_set_size_request(m_searchTagLabel, 40, -1);

  std::string im[] = {"down.png", "up.png", "", ""};
  i = -1;
  for (auto &a : m_button) {
    i++;
    a = gtk_button_new();
    if (oneOf(i, BUTTON_NEXT, BUTTON_PREVIOUS))
      gtk_button_set_image(GTK_BUTTON(a), image(im[i]));
  }
  updateButton(BUTTON_DICTIONARY);

  GtkWidget **widgets[] = {&m_currentDictionary, &m_currentLanguage};

  for (GtkWidget **a : widgets) {
    *a = gtk_label_new("");
    gtk_widget_set_margin_start(*a, 30);
  }

  for (i = 0; i < int(MENU_TO_ACCEL_KEY.size()); i++) {
    m_accelGroup.push_back(gtk_accel_group_new());
  }
  addAccelerators();

  // fill containers
  const int margin = 4;
  add(m_status, m_statusMessage);
  gtk_widget_set_halign(m_statusMessage, GTK_ALIGN_START);
  gtk_widget_set_margin_start(m_statusMessage, TEXT_VIEW_MARGIN);

  w = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
  auto createRow = [&](auto &&...args) {
    GtkWidget *row = createBox(GTK_ORIENTATION_HORIZONTAL, margin,
                               std::forward<decltype(args)>(args)...);
    gtk_container_add(GTK_CONTAINER(w), row);
  };
  createRow(m_button[BUTTON_STARTSTOP], false, m_currentDictionary, false,
            m_button[BUTTON_DICTIONARY], false, m_currentLanguage, false,
            m_button[BUTTON_LANGUAGE], false

  );
  createRow(m_combo[COMBOBOX_SORT], true, m_combo[COMBOBOX_SORT_ORDER], false);
  createRow(m_entry[ENTRY_FILTER], true, m_combo[COMBOBOX_FILTER], false);
  createRow(m_entry[ENTRY_SEARCH], true, m_searchTagLabel, false,
            m_button[BUTTON_NEXT], false, m_button[BUTTON_PREVIOUS], false);

  m_panedWidget = w1 = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
  // left part
  w2 = createBox(GTK_ORIENTATION_VERTICAL, 0, createTextView(TEXTVIEW_MAIN),
                 true, m_status, false);
  gtk_widget_set_size_request(w2, MIN_LEFT_PANEL_WIDTH, -1);
  gtk_paned_pack1(GTK_PANED(w1), w2, FALSE, FALSE);

  // right part
  w2 = createBox(GTK_ORIENTATION_VERTICAL, 3, m_helperUp, false,
                 gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0), true, w, false);
  gtk_widget_set_size_request(w2, MIN_RIGHT_PANEL_WIDTH, -1);
  gtk_paned_pack2(GTK_PANED(w1), w2, TRUE, FALSE);
  gtk_widget_set_margin_start(w2, 5);
  gtk_widget_set_margin_end(w2, 5);

  m_positionSignalId = g_signal_connect(
      w1, "notify::position",
      G_CALLBACK(+[](GObject *object, GParamSpec *pspec, gpointer data) {
        // pr("separator");
        frame->m_separatorPosition = gtk_paned_get_position(GTK_PANED(object));
      }),
      NULL);

  g_signal_handler_block(w1, m_positionSignalId);
  g_timeout_add(50, G_SOURCE_FUNC(+[](gpointer data) -> gboolean {
                  GtkPaned *paned = GTK_PANED(frame->m_panedWidget);
                  gtk_paned_set_position(paned, frame->m_separatorPosition);
                  g_signal_handler_unblock(paned, frame->m_positionSignalId);
                  return G_SOURCE_REMOVE;
                }),
                NULL);

  w = createBox(GTK_ORIENTATION_VERTICAL, 0, m_menu, false, w1, true);
  gtk_container_add(GTK_CONTAINER(m_widget), w);

  // load menu
  i = 0;
  for (auto &s : readFile(m_languageIndex, "language")) {
    if (subMenu.empty() && s.empty()) {
      break;
    }

    if (s.starts_with(SEPARATOR)) {
      gtk_menu_shell_append(
          GTK_MENU_SHELL(gtk_menu_item_get_submenu(subMenu.back())),
          gtk_separator_menu_item_new());
      continue;
    }

    if (s.find('}') != std::string::npos) {
      subMenu.pop_back();
      continue;
    }

    bSubMenu = s.find('{') != std::string::npos;

    if (auto it = MENU_TO_ICON_FILE.get(ENUM_MENU(i))) {
      item = gtk_menu_item_new();
      w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
      w1 = gtk_accel_label_new("");
      gtk_container_add(GTK_CONTAINER(w), image(*it));
      gtk_label_set_use_underline(GTK_LABEL(w1), TRUE);
      gtk_label_set_xalign(GTK_LABEL(w1), 0.0);
      gtk_accel_label_set_accel_widget(GTK_ACCEL_LABEL(w1), item);
      gtk_box_pack_end(GTK_BOX(w), w1, TRUE, TRUE, 0);
      gtk_container_add(GTK_CONTAINER(item), w);
    } else {
      item = gtk_menu_item_new_with_label("");
    }

    gtk_menu_shell_append(
        GTK_MENU_SHELL(subMenu.size() == 0 && bSubMenu
                           ? m_menu
                           : gtk_menu_item_get_submenu(subMenu.back())),
        item);

    if (auto res = MENU_TO_ACCEL_KEY.getWithIndex(ENUM_MENU(i))) {
      auto [val, index] = *res;
      gtk_widget_add_accelerator(item, "activate", m_accelGroup[index], val,
                                 GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    }

    m_menuMap[ENUM_MENU(i)] = item;
    if (bSubMenu) {
      gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), gtk_menu_new());
      subMenu.push_back(GTK_MENU_ITEM(item));
    } else {
      g_signal_connect(item, "activate", G_CALLBACK(menu_activate),
                       GINT_TO_POINTER(i));
    }

    i++;
  }

  updateLanguage();
  setComboIndex(COMBOBOX_SORT, 1);
  setComboIndex(COMBOBOX_FILTER, 0);
  // update menu enables/disables, after language[] is filled
  updateDictionary();

  loadCSS();
  for (i = 0; i < 2; i++) {
    updateFont(ENUM_FONT(i)); // after load css
  }

  for (auto &a : m_button) {
    g_signal_connect(a, "clicked", G_CALLBACK(button_clicked), NULL);
  }
  g_signal_connect(m_combo[COMBOBOX_SORT_ORDER], "changed",
                   G_CALLBACK(combo_changed), gpointer(COMBOBOX_SORT_ORDER));
  g_signal_connect(m_widget, "window-state-event",
                   G_CALLBACK(on_window_state_event), NULL);
  g_signal_connect(m_widget, "delete-event", G_CALLBACK(on_window_delete_event),
                   NULL);
  g_signal_connect(m_widget, "destroy", G_CALLBACK(destroy_window), NULL);

  if (m_maximized) {
    gtk_window_maximize(GTK_WINDOW(m_widget));
  } else {
    gtk_window_set_default_size(GTK_WINDOW(m_widget), m_width, m_height);
    gtk_window_move(GTK_WINDOW(m_widget), m_x, m_y);
  }

  updateStatus(STATE_BEGIN);
  setHelperPanel(false); // after set m_state in updateStatus
  gtk_widget_show_all(m_widget);
  gtk_window_set_focus(GTK_WINDOW(m_widget),
                       NULL); // no focus
  unlockSignals();
  // pr("###################");
}

void Frame::clickMenu(ENUM_MENU menu) {
  GtkTextBuffer *buffer;
  GtkTextIter start, end;
  GtkClipboard *clipboard;
  ENUM_FONT e;
  char *text;
  bool b;

  switch (menu) {

  case MENU_EDIT_SELECT_ALL_AND_COPY_TO_CLIPBOARD:
  case MENU_EDIT_SELECT_ALL:
  case MENU_EDIT_COPY_TO_CLIPBOARD:
    buffer = tvBuffer();
    if (menu != MENU_EDIT_COPY_TO_CLIPBOARD) {
      gtk_text_buffer_get_start_iter(buffer, &start);
      gtk_text_buffer_get_end_iter(buffer, &end);
      gtk_text_buffer_select_range(buffer, &start, &end);
    }

    if (menu != MENU_EDIT_SELECT_ALL) {
      clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
      if (gtk_text_buffer_get_selection_bounds(buffer, &start, &end)) {
        text = gtk_text_buffer_get_text(buffer, &start, &end,
                                        FALSE); // utf8
        gtk_clipboard_set_text(clipboard, text, -1);
        gtk_clipboard_store(clipboard); // available for other applications
        g_free(text);
      }
    }
    break;

  case MENU_SAVE_TEXT:
    saveText();
    break;

  case MENU_FONT_FOR_THE_OUTPUT_WINDOW:
  case MENU_FONT_FOR_THE_CONTROLS:
    e = menu == MENU_FONT_FOR_THE_OUTPUT_WINDOW ? FONT_OUT : FONT_CONTROLS;
    if (selectFont(string(menu), e)) {
      updateFont(e);
    }
    break;

  case MENU_RESET_SETTINGS:
    resetSettings(true);
    break;

  case MENU_LOAD_ENGLISH_DICTIONARY:
  case MENU_LOAD_RUSSIAN_DICTIONARY:
    updateDictionary(true);
    break;

  case MENU_ABOUT:
    aboutDialog();
    break;

  case MENU_HOMEPAGE:
    openURL(HOMEPAGE);
    break;

  case MENU_SOURCE_CODE:
    openURL(SOURCE_URL);
    break;

  default:
    if (oneOf(menu, MENU_ENGLISH_LANGUAGE, MENU_RUSSIAN_LANGUAGE)) {
      b = m_menuClick != MENU_SEARCH;
      updateLanguage(true); // helper panel update incilde
    } else {
      m_menuClick = menu;
      b = m_menuClick != MENU_SEARCH;
      setHelperPanel(b);
    }
    // pr(magic_enum::enum_name(m_menuClick));
    if (b) {
      job();
    }
  }
}

void Frame::destroy() {
  writeConfig(CONFIG_TAGS, WORDS_VERSION, m_languageIndex, m_dictionaryIndex,
              m_separatorPosition, m_font[0].get(), m_font[1].get(),
              m_maximized, m_x, m_y, m_width, m_height);
  job(JOB_TYPE_STOP);
  gtk_main_quit();
}

void Frame::aboutDialog() {
  size_t i;
  int h;
  std::string s, s1;
  GtkWidget *box, *hbox, *dialog, *label, *img = gtk_image_new();
  char *markup;
  GdkPixbuf *pi;

  dialog = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(dialog), getMenuLabel(MENU_ABOUT).c_str());
  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_container_add(GTK_CONTAINER(hbox), img);
  box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

  ENUM_STRING sid[] = {PROGRAM,
                       AUTHOR,
                       COPYRIGHT,
                       HOMEPAGE_STRING,
                       HOMEPAGE_ONLINE_STRING,
                       SOURCE_CODE,
                       STRING_SIZE /*build info*/,
                       EXECUTABLE_FILE_SIZE};
  for (auto id : sid) {
    if (auto key_opt = MAP_URL.get(id)) {
      s = *key_opt + (m_languageIndex && id != SOURCE_CODE
                          ? ',' + LNG[m_languageIndex]
                          : "");
      label = gtk_label_new(NULL);
      markup =
          g_markup_printf_escaped("%s <a href=\"%s\">\%s</a>",
                                  string(id).c_str(), s.c_str(), s.c_str());
      gtk_label_set_markup(GTK_LABEL(label), markup);
      g_free(markup);
      g_signal_connect(label, "activate-link", G_CALLBACK(label_clicked),
                       gpointer(NULL));
    } else {
      if (id == PROGRAM) {
        s = getProgramVersionString();
#ifndef NDEBUG
        // output that NDEBUG is not defined
        s += " DEBUG VERSION";
#endif
      } else if (id == STRING_SIZE) {
        s = getBuildVersionString(false);
      } else {
        s = string(id);
        if (id == AUTHOR) {
          s += " " + string(EMAIL_STRING) + " " + MAIL;
        } else if (id == COPYRIGHT) {
          i = s.find('(');
          if (i != std::string::npos) {
            s = s.substr(0, i + 1) + "\u00A9" + s.substr(i + 2);
          }
        } else if (id == EXECUTABLE_FILE_SIZE) {
          s += " " + toString(getApplicationFileSize(), ',');
        }
      }
      label = createLabel(s);
    }
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    add(box, label); // stretch vertically
  }

  gtk_container_add(GTK_CONTAINER(hbox), box);
  gtk_container_add(
      GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), hbox);

  gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
  gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(m_widget));

  gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
  gtk_widget_show_all(dialog);

  // get height after show_all
  gtk_widget_get_preferred_height(box, nullptr, &h);
  pi = gdk_pixbuf_new_from_file_at_size(getImagePath("word256.png").c_str(), h,
                                        h, 0);
  gtk_image_set_from_pixbuf(GTK_IMAGE(img), pi);
  g_object_unref(pi);

  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
}

void Frame::setHelperPanel(bool ignoreStateBegin) {
  int i;
  GtkWidget *w;
  std::string s;

  clearContainer(m_helperUp);
  m_charactersLabel = nullptr;

  if (!ignoreStateBegin &&
      m_state == STATE_BEGIN) { // also worked when language is changed
    addHelp(STARTMESSAGE);
    gtk_widget_show_all(m_helperUp);
    return;
  }

  // set label
  if (auto it = MENU_WITH_HELP_STRING.get(m_menuClick)) {
    addHelp(*it);
  }

  if (auto it = MENU_WITH_ENTRY.get(m_menuClick)) {
    w = createBox(GTK_ORIENTATION_HORIZONTAL, 3, MENU_SEARCH, false,
                  createEntry(ENTRY_TEMPLATE, *it), true);
    gtk_container_add(GTK_CONTAINER(m_helperUp), w);
  }

  if (auto it = FROM_TO_COMBO.get(m_menuClick)) {
    addComboLineToHelper(string(it->s1, it->s2), it->min,
                         it->max[m_dictionaryIndex], it->active, it->send);
  }

  switch (m_menuClick) {

  case MENU_TEMPLATE:
    addComboToHelper(TEMPLATE1, TEMPLATE3, 0);
    break;

  case MENU_REGULAR_EXPRESSIONS:
    addComboLineToHelper(NUMBER_OF_MATCHES, 1, 10, 0, STRING_SIZE, true);
    break;

  case MENU_MODIFICATION:
    createCheck(EVERY_MODIFICATION_CHANGES_WORD, TRUE);
    gtk_container_add(GTK_CONTAINER(m_helperUp), m_check);
    break;

  case MENU_CHAIN:
    i = -1;
    for (auto a :
         {createLabel(EXCEPTION_WORDS),
          createTextView(TEXTVIEW_HELPER, SETTINGS_CHAIN_EXCEPTIONS)}) {
      i++;
      if (!i)
        gtk_widget_set_halign(a, GTK_ALIGN_START);
      gtk_container_add(GTK_CONTAINER(m_helperUp), a);
    }
    break;

  case MENU_CHARACTER_SEQUENCE:
    addComboToHelper(SEARCH_IN_ANY_PLACE_OF_WORD, SEARCH_IN_END_OF_WORD, 0,
                     COMBOBOX_HELPER2);
    addComboLineToHelper(NUMBER_OF_MATCHES, 1, 10, 0, STRING_SIZE, true);
    break;

  case MENU_WORDS_SPLIT:
    m_charactersLabel = createLabel("");
    w = createBox(GTK_ORIENTATION_HORIZONTAL, COMBOLINE_MARGIN,
                  string(LENGTH, OF_WORD), true,
                  createTextCombo(COMBOBOX_HELPER0, 6, 20, 7 - 6), true,
                  m_charactersLabel, true);
    gtk_container_add(GTK_CONTAINER(m_helperUp), w);
    break;

  case MENU_CONSONANT_VOWEL_SEQUENCE:
    w = createBox(GTK_ORIENTATION_HORIZONTAL, COMBOLINE_MARGIN,
                  createTextCombo(COMBOBOX_HELPER0, SEARCH_IN_ANY_PLACE_OF_WORD,
                                  SEARCH_IN_END_OF_WORD, 0),
                  true,

                  createTextCombo(COMBOBOX_HELPER1, 3, 10, 0), true,
                  createTextCombo(COMBOBOX_HELPER2, VOWELS, CONSONANTS, 0),
                  true);
    gtk_container_add(GTK_CONTAINER(m_helperUp), w);
    break;

  case MENU_DENSITY:
    w = createBox(GTK_ORIENTATION_HORIZONTAL, COMBOLINE_MARGIN, MAXIMUM, true,
                  createTextCombo(COMBOBOX_HELPER0, 0, 25, 25), true, "%", true,
                  createTextCombo(COMBOBOX_HELPER1, VOWELS, CONSONANTS, 0),
                  true, CHARACTERS, true);
    gtk_container_add(GTK_CONTAINER(m_helperUp), w);
    break;
  default:;
  }

  if (m_charactersLabel) {
    updateCharactersLabel();
  }

  gtk_widget_show_all(m_helperUp);
}

void Frame::updateDictionary(bool change) {
  if (change) {
    m_dictionaryIndex = !m_dictionaryIndex;
  }
  updateSensitivity([this](int i) { return m_dictionaryIndex != i; },
                    MENU_LOAD_ENGLISH_DICTIONARY, MENU_LOAD_RUSSIAN_DICTIONARY);
  updateButton(BUTTON_DICTIONARY);

  if (auto it = MENU_WITH_ENTRY.get(m_menuClick)) {
    auto e = ENTRY_TEMPLATE;
    std::string s = stringUsingDictionary(*it, true);
    s = normalizeString(e, s);
    // if user didn't change value of enty we can set it by default
    if (s == m_entryValue[e]) {
      s = stringUsingDictionary(*it);
      gtk_entry_set_text(GTK_ENTRY(m_entry[e]), s.c_str());
      updateEntryValue(e);
    }
  }

  if (auto it = FROM_TO_COMBO.get(m_menuClick)) {
    for (auto a : {COMBOBOX_HELPER0, COMBOBOX_HELPER1}) {
      refillCombo(a, it->min, it->max[m_dictionaryIndex] /*, it->active*/);
    }
  }

  if (m_menuClick != MENU_SEARCH) {
    job();
  }
}

void Frame::updateLanguage(bool change) {
  if (change) {
    m_languageIndex = !m_languageIndex;
  }
  std::string s;
  int i = -1;
  for (auto &a : m_menuAll[m_languageIndex]) {
    i++;
    setMenuLabel(ENUM_MENU(i), a);
  }

  updateSensitivity([this](int i) { return m_languageIndex != i; },
                    MENU_ENGLISH_LANGUAGE, MENU_RUSSIAN_LANGUAGE);

  setPlaceholder(ENTRY_SEARCH, MENU_SEARCH);
  setLabel(m_currentDictionary, DICTIONARY);
  setLabel(m_currentLanguage, MENU_LANGUAGE);
  updateButton(BUTTON_LANGUAGE);
  gtk_window_set_title(GTK_WINDOW(m_widget), string(PROGRAM).c_str());
  setPlaceholder(ENTRY_FILTER, RESULTS_FILTER);

  refillCombo(COMBOBOX_SORT, SORT_BY_ALPHABET,
              SORT_BY_DIFFERENT_NUMBER_OF_CHARACTERS);
  refillCombo(COMBOBOX_FILTER, FOUND, NOT_FOUND);
  setHelperPanel(m_state != STATE_BEGIN);
}

void Frame::refillCombo(ENUM_COMBOBOX e, const VString &v, int active) {
  if (active == -1) {
    active = getComboIndex(e);
  }
  if (active >= int(v.size())) {
    active = v.size() - 1;
  }
  bool b = isSignalsLocked();
  if (!b)
    lockSignals();
  auto c = GTK_COMBO_BOX_TEXT(m_combo[e]);
  gtk_combo_box_text_remove_all(c);
  for (auto &a : v) {
    gtk_combo_box_text_append_text(c, a.c_str());
  }
  setComboIndex(e, active);
  if (!b)
    unlockSignals();
}

GtkWidget *Frame::createTextCombo(ENUM_COMBOBOX e, VString v, int active) {
  GtkWidget *w = m_combo[e] = gtk_combo_box_text_new();
  refillCombo(e, v, active);
  g_signal_connect(w, "changed", G_CALLBACK(combo_changed), gpointer(e));
  return w;
}

void Frame::addComboLineToHelper(int from, int to, int active, std::string s1,
                                 std::string s2, std::string s3, bool any) {
  int i, j;
  GtkWidget *w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, COMBOLINE_MARGIN), *r;
  for (i = 0; i < 2; i++) {
    add(w, i == 0 ? s1 + (any ? "" : " " + string(FROM)) : s2);
    if (!i && any) {
      for (j = 0; j < 2; j++) {
        if (j == 0) {
          m_radio = r =
              gtk_radio_button_new_with_label(NULL, string(ANY).c_str());
        } else {
          r = gtk_radio_button_new_with_label_from_widget(
              GTK_RADIO_BUTTON(m_radio), string(FROM).c_str());
        }
        add(w, r);
        g_signal_connect(r, "toggled", G_CALLBACK(radio_changed), NULL);
      }
      updateRadioValue();
    }
    add(w,
        createTextCombo(ENUM_COMBOBOX(COMBOBOX_HELPER0 + i), from, to, active));
  }
  if (!s3.empty()) {
    m_charactersLabel = createLabel(s3);
    add(w, m_charactersLabel);
  }
  gtk_container_add(GTK_CONTAINER(m_helperUp), w);
  m_comboline = w;
}

void Frame::addComboToHelper(ENUM_STRING from, ENUM_STRING to, int active,
                             ENUM_COMBOBOX comboboxId /*=COMBOBOX_HELPER0*/) {
  gtk_container_add(GTK_CONTAINER(m_helperUp),
                    createTextCombo(comboboxId, from, to, active));
}

void Frame::comboChanged(ENUM_COMBOBOX e) {
  assert(e != COMBOBOX_SIZE);
  updateComboValue(e);
  bool regexOrCharSequence =
      oneOf(m_menuClick, MENU_REGULAR_EXPRESSIONS, MENU_CHARACTER_SEQUENCE);

  if (m_menuClick == MENU_CHARACTER_SEQUENCE && e == COMBOBOX_HELPER2) {
    // visible is "any place of word"
    gtk_widget_set_visible(m_comboline, getComboIndex(e) == 0);
  }

  if (oneOf(e, COMBOBOX_SORT, COMBOBOX_SORT_ORDER, COMBOBOX_FILTER)) {
    // pr(magic_enum::enum_name(e));
    job(e == COMBOBOX_FILTER ? JOB_TYPE_FILTER : JOB_TYPE_SORT_AND_FILTER);
    return;
  }
  if (getLastCombobox() == e) {
    updateCharactersLabel();
  }

  if ((e == COMBOBOX_HELPER0 || e == COMBOBOX_HELPER1) &&
      (regexOrCharSequence || FROM_TO_COMBO.has(m_menuClick))) {
    if (getComboIndex(COMBOBOX_HELPER0) > getComboIndex(COMBOBOX_HELPER1)) {
      lockSignals();
      setComboIndex(e == COMBOBOX_HELPER0 ? COMBOBOX_HELPER1 : COMBOBOX_HELPER0,
                    getComboIndex(e));
      unlockSignals();
    }
  }

  if (regexOrCharSequence &&
      getSelectedRadioIndex() ==
          0) { // any number of matches skip combo not used
  } else {
    job();
  }
}

void Frame::createImageCombo(ENUM_COMBOBOX e, int active) {
  int i;
  GtkTreeIter iter;
  GtkCellRenderer *renderer;
  const char *image[] = {"ascending.png", "descending.png"};
  GtkListStore *gls = gtk_list_store_new(1, GDK_TYPE_PIXBUF);
  for (i = 0; i < 2; i++) {
    gtk_list_store_append(gls, &iter);
    gtk_list_store_set(gls, &iter, 0, pixbuf(image[i]), -1);
  }
  m_combo[e] = gtk_combo_box_new_with_model(GTK_TREE_MODEL(gls));
  renderer = gtk_cell_renderer_pixbuf_new();
  gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(m_combo[e]), renderer, TRUE);
  gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(m_combo[e]), renderer,
                                 "pixbuf", 0, NULL);
  setComboIndex(e, active);
}

void Frame::clickButton(GtkWidget *button) {
  int i;
  ENUM_BUTTON e = ENUM_BUTTON(indexOf(button, m_button));
  if (oneOf(e, BUTTON_DICTIONARY, BUTTON_STARTSTOP, BUTTON_LANGUAGE)) {
    auto now = std::chrono::steady_clock::now();
    if (now - last_click_time[e] < std::chrono::milliseconds(TIMER_BUTTON)) {
      return;
    }
    last_click_time[e] = now;

    if (e == BUTTON_DICTIONARY) {
      updateDictionary(true);
    } else if (e == BUTTON_STARTSTOP) {
      auto b = getStartStopState();
      job(b.imageStart ? JOB_TYPE_FULL : JOB_TYPE_STOP);
    } else if (e == BUTTON_LANGUAGE) {
      updateLanguage(true);
    }
  } else {
    if (m_tags < 2) {
      return;
    }
    i = m_tagIndex + (e == BUTTON_NEXT ? 1 : m_tags - 1);
    updateTags(i % m_tags);
  }
}

void Frame::setMenuLabel(ENUM_MENU e, std::string const &text) {
  GtkWidget *w = m_menuMap[e];
  if (MENU_TO_ICON_FILE.has(e)) {
    w = gtk_bin_get_child(GTK_BIN(w));
    GList *list = gtk_container_get_children(GTK_CONTAINER(w));
    assert(g_list_length(list) == 2);

    gtk_label_set_label(GTK_LABEL(g_list_nth(list, 1)->data), text.c_str());
  } else {
    gtk_menu_item_set_label(GTK_MENU_ITEM(w), text.c_str());
  }
}

std::string Frame::getMenuLabel(ENUM_MENU e) {
  GtkWidget *w = m_menuMap[e];
  if (MENU_TO_ICON_FILE.has(e)) {
    w = gtk_bin_get_child(GTK_BIN(w));
    GList *list = gtk_container_get_children(GTK_CONTAINER(w));
    assert(g_list_length(list) == 2);
    return gtk_label_get_label(GTK_LABEL(g_list_nth(list, 1)->data));
  } else {
    return gtk_menu_item_get_label(GTK_MENU_ITEM(w));
  }
}

void Frame::endJob() {
  // prsync("endJob");
  //  make unjoinable
  if (m_thread.joinable())
    m_thread.join();
  updateStatus(m_token.stop_requested() ? STATE_USER_BREAK : STATE_OK);
  // update tags if user searched something
  updateTags(0);
  if (m_currentEntry !=
      ENTRY_SIZE) { // means calls from entry changed,restor focus and position
    gtk_widget_grab_focus(m_entry[m_currentEntry]);
    gtk_editable_set_position(GTK_EDITABLE(m_entry[m_currentEntry]),
                              m_currentEntryPos);
    m_currentEntry = ENTRY_SIZE; // reset value
  }
}

int Frame::getComboIndex(ENUM_COMBOBOX e) const {
  assert(e != COMBOBOX_SIZE);
  assert(GTK_IS_COMBO_BOX(m_combo[e]));
  return gtk_combo_box_get_active(GTK_COMBO_BOX(m_combo[e]));
}

void Frame::setComboIndex(ENUM_COMBOBOX e, int v) {
  assert(GTK_IS_COMBO_BOX(m_combo[e]));
  gtk_combo_box_set_active(GTK_COMBO_BOX(m_combo[e]), v);
  updateComboValue(e);
}

void Frame::updateComboValue(ENUM_COMBOBOX e) {
  assert(e != COMBOBOX_SIZE);
  int v = -1;
  if (GTK_IS_COMBO_BOX_TEXT(m_combo[e])) {
    char *p =
        gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(m_combo[e]));
    if (p && parseString(p, v)) {
      assert(v >= 0);
    }
  }
  if (v == -1) {
    v = getComboIndex(e);
    // v can be =-1 when combobox just created
  }

  // print_short_stack_trace( 6);

  m_comboValue[e] = v;
}

void Frame::entryFocusChanged(bool in) {
  if (in) {
    removeAccelerators();
  } else {
    addAccelerators();
  }
}

void Frame::removeAccelerators() {
  for (auto &a : m_accelGroup) {
    gtk_window_remove_accel_group(GTK_WINDOW(m_widget), a);
  }
}

/**
 * Note even if program use accelerators and combobox is active then hotkeys
 * don't work
 */
void Frame::addAccelerators() {
  for (auto &a : m_accelGroup) {
    gtk_window_add_accel_group(GTK_WINDOW(m_widget), a);
  }
}

void Frame::newVersionMessage() {
  std::string s = string(NEW_VERSION_MESSAGE) + "\n" + m_newVersion.m_message;
  GtkWidget *d =
      gtk_message_dialog_new(GTK_WINDOW(m_widget), GTK_DIALOG_MODAL,
                             GTK_MESSAGE_INFO, GTK_BUTTONS_YES_NO, s.c_str());

  gtk_window_set_title(GTK_WINDOW(d), getProgramVersionString().c_str());
  gint result = gtk_dialog_run(GTK_DIALOG(d));
  gtk_widget_destroy(d);

  if (result == GTK_RESPONSE_YES) {
    openURL(DOWNLOAD_URL);
  }
}

void Frame::setDebounceTimer(ENUM_ENTRY e) {
  if (m_debounceTimerId) {
    g_source_remove(m_debounceTimerId);
  }
  m_debounceTimerId =
      g_timeout_add(TIMER, on_debounce_timeout, GINT_TO_POINTER(e));
}

void Frame::debounceTimeout(ENUM_ENTRY e) {
  m_debounceTimerId = 0;
  // store current entry, because after thread loose focus and cursor position
  m_currentEntry = e;
  m_currentEntryPos = gtk_editable_get_position(GTK_EDITABLE(m_entry[e]));

  switch (e) {
  case ENTRY_TEMPLATE:
    job();
    break;

  case ENTRY_SEARCH:
    updateTags(0);
    break;

  case ENTRY_FILTER:
    if (m_regex[ENTRY_FILTER]) {
      job(JOB_TYPE_FILTER);
    }
    break;

  default:
    assert(0);
  }
}

void Frame::entryChanged(ENUM_ENTRY e) {
  bool b;
  // pr(magic_enum::enum_name(e));
  updateEntryValue(e);
  if (e == ENTRY_TEMPLATE) {
    b = prepare();
    addRemoveClass(m_entry[e], CERROR, !b);
  } else if (e == ENTRY_FILTER) {
    clearTagMarks();
    b = createRegex(e);
    addRemoveClass(m_entry[e], CERROR, !b);
  } else if (e == ENTRY_SEARCH) {
    clearTagMarks();
  }

  if (!isSignalsLocked()) {
    setDebounceTimer(e);
  }
}

void Frame::clearTagMarks() {
  GtkTextBuffer *buffer = tvBuffer();
  for (auto &range : m_found_tags) {
    gtk_text_buffer_delete_mark(buffer, range.start_mark);
    gtk_text_buffer_delete_mark(buffer, range.end_mark);
  }
  m_found_tags.clear();
}

// n - number of active tag
void Frame::updateTags(int n) {
  GtkTextBuffer *buffer = tvBuffer();
  std::string s = m_entryValue[ENTRY_SEARCH];

  int old_tag_index = m_tagIndex;
  m_tagIndex = n;

  if (!s.empty() && !m_found_tags.empty() && m_tags > 0) {

    if (old_tag_index >= 0 && old_tag_index < (int)m_found_tags.size()) {
      GtkTextIter s_iter, e_iter;
      gtk_text_buffer_get_iter_at_mark(buffer, &s_iter,
                                       m_found_tags[old_tag_index].start_mark);
      gtk_text_buffer_get_iter_at_mark(buffer, &e_iter,
                                       m_found_tags[old_tag_index].end_mark);

      gtk_text_buffer_remove_tag_by_name(buffer, activeTag, &s_iter, &e_iter);
      gtk_text_buffer_apply_tag_by_name(buffer, markTag, &s_iter, &e_iter);
    }

    GtkTextIter scroll;
    if (m_tagIndex >= 0 && m_tagIndex < (int)m_found_tags.size()) {
      GtkTextIter s_iter, e_iter;
      gtk_text_buffer_get_iter_at_mark(buffer, &s_iter,
                                       m_found_tags[m_tagIndex].start_mark);
      gtk_text_buffer_get_iter_at_mark(buffer, &e_iter,
                                       m_found_tags[m_tagIndex].end_mark);

      gtk_text_buffer_remove_tag_by_name(buffer, markTag, &s_iter, &e_iter);
      gtk_text_buffer_apply_tag_by_name(buffer, activeTag, &s_iter, &e_iter);
      scroll = s_iter;
    }

    gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(m_text[TEXTVIEW_MAIN]), &scroll,
                                 0.0, true, .5, .5);

    setLabel(m_searchTagLabel,
             std::format("{}/{}", m_tags == 0 ? 0 : m_tagIndex + 1, m_tags));
    return;
  }

  GtkTextIter first, scroll, start, end;
  gint i, j;

  gtk_text_buffer_get_bounds(buffer, &start, &end);
  gtk_text_buffer_remove_all_tags(buffer, &start, &end);
  clearTagMarks();

  if (s.empty()) {
    setLabel(m_searchTagLabel, "");
    return;
  }

  VString v = split(gtk_text_iter_get_text(&start, &end), "\n");
  const gchar *text = s.c_str();

  gtk_text_buffer_get_start_iter(buffer, &first);
  for (m_tags = 0; gtk_text_iter_forward_search(
           &first, text,
           GtkTextSearchFlags(GTK_TEXT_SEARCH_TEXT_ONLY |
                              GTK_TEXT_SEARCH_CASE_INSENSITIVE |
                              GTK_TEXT_SEARCH_VISIBLE_ONLY),
           &start, &end, NULL);
       gtk_text_buffer_get_iter_at_offset(buffer, &first,
                                          gtk_text_iter_get_offset(&end))) {

    i = gtk_text_iter_get_line(&start);
    j = gtk_text_iter_get_line_offset(&start);
    auto p = v[i].find(OPEN_BRACKET);
    if (p != std::string::npos) {
      i = g_utf8_strlen(v[i].c_str(), p);
      if (j >= i) {
        continue;
      }
    }

    GtkTextMark *sm = gtk_text_buffer_create_mark(buffer, NULL, &start, TRUE);
    GtkTextMark *em = gtk_text_buffer_create_mark(buffer, NULL, &end, FALSE);
    m_found_tags.push_back({sm, em});

    if (m_tags == m_tagIndex) {
      scroll = start;
    }

    gtk_text_buffer_apply_tag_by_name(
        buffer, m_tags == m_tagIndex ? activeTag : markTag, &start, &end);
    m_tags++;
  }

  if (m_tags > 0) {
    gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(m_text[TEXTVIEW_MAIN]), &scroll,
                                 0.0, true, .5, .5);
  }
  setLabel(m_searchTagLabel,
           std::format("{}/{}", m_tags == 0 ? 0 : m_tagIndex + 1, m_tags));
}

std::string Frame::getProgramVersionString() const {
  return string(PROGRAM, VERSION) + " " + WORDS_VERSION;
}

void Frame::updateStatus(ENUM_STATE state) {
  // prsync(magic_enum::enum_name(state));
  m_state = state;
  bool b = true;
  switch (state) {
  case STATE_BEGIN:
    m_out = "";
    break;

  case STATE_OK:
    if (SearchResult::out.size()) {
      // SearchResult::out can be too big so can't copy
      // m_out = std::move(SearchResult::out); also not possible need to store
      // SearchResult::out
      b = false;
    } else {
      m_out = string(NO_RESULTS_FOUND);
    }
    break;

  case STATE_PROCEEDING:
    m_out = oneOf(m_menuClick, MENU_WAITING) ? string(WAITING)
                                             : string(MENU_SEARCH);
    break;

  case STATE_ERROR:
    m_out = string(STRING_ERROR);
    break;

  case STATE_USER_BREAK:
    m_out = string(OPERATION_CANCELED_BY_USER);
    break;
  }

  std::string s = state == STATE_OK ? getStatusString() : m_out;
  setLabel(m_statusMessage, s);

  if (b && state != STATE_BEGIN) {
    m_out =
        capitalizeFirstUtf8(m_out) + (m_state == STATE_PROCEEDING ? "…" : ".");
  }
  updateTextView(TEXTVIEW_MAIN, b ? m_out : SearchResult::out);

  b = state == STATE_OK && !m_result.empty();
  updateSensitivity(b, COMBOBOX_SORT, COMBOBOX_SORT_ORDER, COMBOBOX_FILTER,
                    ENTRY_FILTER);
  updateButton(BUTTON_STARTSTOP);
}

void Frame::updateButton(ENUM_BUTTON e) {
  std::string s;
  if (e == BUTTON_STARTSTOP) {
    auto b = getStartStopState();
    updateSensitivity(b.enable, e);
    s = b.imageStart ? "play.png" : "stop.png";

  } else if (e == BUTTON_DICTIONARY) {
    s = m_dictionaryIndex ? "ru.gif" : "en.gif";
  } else if (e == BUTTON_LANGUAGE) {
    s = m_languageIndex ? "ru.gif" : "en.gif";
  }

  gtk_button_set_image(GTK_BUTTON(m_button[e]), image(s));
}

StartStopButtonState Frame::getStartStopState() const {
  return {m_state != STATE_PROCEEDING,
          !oneOf(m_state, STATE_ERROR, STATE_BEGIN)};
}

GtkWidget *Frame::createTextView(ENUM_TEXTVIEW e, ENUM_STRING n) {
  std::string s;
  auto t = m_text[e] = gtk_text_view_new();
  auto w = gtk_scrolled_window_new(NULL, NULL);
  gtk_container_add(GTK_CONTAINER(w), t);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(t), GTK_WRAP_WORD);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(w), GTK_POLICY_AUTOMATIC,
                                 GTK_POLICY_AUTOMATIC);

  GtkTextBuffer *buffer = tvBuffer(e);
  if (e == TEXTVIEW_MAIN) {
    gtk_text_view_set_editable(GTK_TEXT_VIEW(t), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(t), FALSE);
    gtk_text_buffer_create_tag(buffer, markTag, "background", "lightblue",
                               NULL);
    gtk_text_buffer_create_tag(buffer, activeTag, "background", "Khaki", NULL);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(t), TEXT_VIEW_MARGIN);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(t), TEXT_VIEW_MARGIN);
    gtk_text_buffer_set_text(buffer, s.c_str(), -1);
  } else if (e == TEXTVIEW_HELPER) {
    s = stringUsingDictionary(n);
    gtk_text_buffer_set_text(buffer, s.c_str(), -1);
    updateTextviewValue(e);
    g_signal_connect(buffer, "changed", G_CALLBACK(textview_changed),
                     GINT_TO_POINTER(TEXTVIEW_HELPER));
  } else {
    assert(0);
  }
  return w;
}

void Frame::textviewChanged(ENUM_TEXTVIEW e) {
  if (e == TEXTVIEW_HELPER) {
    updateTextviewValue(e);
    if (!isSignalsLocked()) {
      // proceed same as template entry changed
      setDebounceTimer(ENTRY_TEMPLATE);
    }
  }
}

void Frame::updateTextviewValue(ENUM_TEXTVIEW e) {
  if (e == TEXTVIEW_HELPER) {
    m_textViewValue = getTextViewString(e, true);
  }
}

std::string Frame::getTextViewString(ENUM_TEXTVIEW e, bool locale) const {
  GtkTextBuffer *buffer = tvBuffer(e);
  GtkTextIter start, end;
  std::string s;
  gtk_text_buffer_get_bounds(buffer, &start, &end);
  gchar *raw_text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
  s = locale ? utf8ToLocale(raw_text) : raw_text;
  g_free(raw_text);
  return s;
}

GtkTextBuffer *Frame::tvBuffer(ENUM_TEXTVIEW e) const {
  return gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_text[e]));
}

void Frame::updateTextView(ENUM_TEXTVIEW e, std::string const &s) {
  gtk_text_buffer_set_text(tvBuffer(e), s.c_str(), -1);
  if (e == TEXTVIEW_MAIN) {
    updateSensitivity(m_state == STATE_OK && !s.empty(), MENU_SAVE_TEXT);
  } else {
    updateTextviewValue(e);
  }
}

GtkWidget *Frame::createEntry(ENUM_ENTRY e, ENUM_STRING n) {
  m_entry[e] = gtk_entry_new();
  std::string s = n == STRING_SIZE ? "" : stringUsingDictionary(n);
  if (e == ENTRY_TEMPLATE) {
    gtk_entry_set_text(GTK_ENTRY(m_entry[e]), s.c_str());
  }
  updateEntryValue(e);

  const std::pair<const char *, GCallback> p[] = {
      {"insert-text", G_CALLBACK(entry_insert)},
      {"delete-text", G_CALLBACK(entry_delete)},
      {"focus-in-event", G_CALLBACK(entry_focus_in)},
      {"focus-out-event", G_CALLBACK(entry_focus_out)}};

  for (auto a : p) {
    g_signal_connect_after(G_OBJECT(m_entry[e]), a.first, a.second,
                           GINT_TO_POINTER(e));
  }
  return m_entry[e];
}

void Frame::updateEntryValue(ENUM_ENTRY e) {
  std::string s = gtk_entry_get_text(GTK_ENTRY(m_entry[e]));
  m_entryValue[e] = normalizeString(e, s);
}

std::string Frame::normalizeString(ENUM_ENTRY e, std::string const &q) {
  std::string s = q;
  // lowercased utf8, changed 'ё' -> 'е'
  bool regex = e == ENTRY_FILTER ||
               (e == ENTRY_TEMPLATE && m_menuClick == MENU_REGULAR_EXPRESSIONS);
  if (regex) {
    s = lowercase_utf8_regex(s);
  } else {
    s = utf8ToLowerCase(s);
  }
  return replaceAll(s, "ё", "е");
}

void Frame::updateCharactersLabel() {
  bool b;
  const int n = m_comboValue[getLastCombobox()];
  const int mod100 = n % 100;
  const int mod10 = n % 10;
  // in the genitive case [ru] в родительном падеже
  ENUM_STRING e;
  if (m_languageIndex == 0) {
    b = n == 1;
  } else {
    b = (mod100 < 10 || mod100 > 20) && mod10 == 1;
  }
  e = b ? CHARACTERS1 : CHARACTERS;
  gtk_label_set_text(GTK_LABEL(m_charactersLabel), string(e).c_str());
}

ENUM_COMBOBOX Frame::getLastCombobox() {
  // use only for MENU_WORDS_SPLIT..., to update characterslabel
  if (m_menuClick == MENU_WORDS_SPLIT)
    return COMBOBOX_HELPER0;
  if (oneOf(m_menuClick, MENU_ANAGRAM, MENU_SIMPLE_WORD_SEQUENCE,
            MENU_DOUBLE_WORD_SEQUENCE))
    return COMBOBOX_HELPER1;
  return COMBOBOX_SIZE;
}

bool Frame::selectFont(const std::string &s, ENUM_FONT e) {
  auto &font = m_font[e];
  GtkWidget *dialog =
      gtk_font_chooser_dialog_new(s.c_str(), GTK_WINDOW(m_widget));
  gtk_font_chooser_set_font_desc(GTK_FONT_CHOOSER(dialog), font.get());
  gint result = gtk_dialog_run(GTK_DIALOG(dialog));
  bool r = (result == GTK_RESPONSE_OK || result == GTK_RESPONSE_APPLY);

  if (r) {
    const PangoFontDescription *new_font =
        gtk_font_chooser_get_font_desc(GTK_FONT_CHOOSER(dialog));
    if (new_font) {
      font.reset(pango_font_description_copy(new_font));
    }
  }

  gtk_widget_destroy(dialog);
  return r;
}

std::string Frame::getCssFromPango(ENUM_FONT e) {
  auto &font = m_font[e];
  if (!font)
    return "";

  // 1. Extract the family name
  const char *family = pango_font_description_get_family(font.get());
  std::string family_name = family ? family : "Monospace";

  // 2. Extract the font size
  double size = pango_font_description_get_size(font.get()) /
                static_cast<double>(PANGO_SCALE);

  // 3. Detect Style (Italic / Oblique)
  std::string font_style = "normal";
  PangoStyle style = pango_font_description_get_style(font.get());
  if (style == PANGO_STYLE_ITALIC || style == PANGO_STYLE_OBLIQUE) {
    font_style = "italic";
  }

  // 4. Detect Weight (Bold)
  std::string font_weight = "normal";
  PangoWeight weight = pango_font_description_get_weight(font.get());
  if (weight >= PANGO_WEIGHT_BOLD) {
    font_weight = "bold"; // Maps to standard CSS bold
  }

  return std::format(
      R"({} {{
    font-family: "{}";
    font-size: {}pt;
    font-style: {};
    font-weight: {};
}})",
      e == FONT_OUT ? "textview" : "label,combobox,entry", family_name, size,
      font_style, font_weight);
}

void Frame::updateFont(ENUM_FONT e) { loadCSS(getCssFromPango(e)); }

void Frame::resetSettings(bool update) {
  int oldLanguage = m_languageIndex;
  int oldDictoinary = m_dictionaryIndex;
  m_languageIndex = m_dictionaryIndex = getSystemLanguage() == "ru";
  m_separatorPosition = DEFAULT_SEPARATOR_POSITION;
  m_maximized = true;
  m_x = m_y = m_width = m_height = 0;
  int i = 0;
  for (auto a : {"Monospace 14px", "Tahoma 14px"}) {
    m_font[i++].reset(pango_font_description_from_string(a));
  }

  if (!update)
    return;

  for (i = 0; i < FONT_SIZE; i++) {
    updateFont(ENUM_FONT(i));
  }
  if (oldDictoinary != m_dictionaryIndex)
    updateDictionary();
  if (oldLanguage != m_languageIndex)
    updateLanguage();

  gtk_paned_set_position(GTK_PANED(m_panedWidget), m_separatorPosition);
}

void Frame::saveText() {
  GtkWidget *dialog;
  GtkFileChooser *chooser;
  gint res;
  std::string s, s1;
  dialog = gtk_file_chooser_dialog_new(
      string(MENU_SAVE_TEXT).c_str(), GTK_WINDOW(m_widget),
      GTK_FILE_CHOOSER_ACTION_SAVE, string(CANCEL).c_str(), GTK_RESPONSE_CANCEL,
      string(SAVE).c_str(), GTK_RESPONSE_ACCEPT, NULL);

  chooser = GTK_FILE_CHOOSER(dialog);
  gtk_file_chooser_set_do_overwrite_confirmation(chooser, TRUE);
  gtk_file_chooser_set_current_name(chooser, "untitled.txt");
  // gtk_file_chooser_set_current_folder(chooser, "/home/user/Documents");

  for (auto a : {TEXT_FILES, ALL_FILES}) {
    GtkFileFilter *filter = gtk_file_filter_new();
    s1 = std::format("*.{}", a == TEXT_FILES ? "txt" : "*");
    s = std::format("{} ({})", string(a), s1);
    gtk_file_filter_set_name(filter, s.c_str());
    gtk_file_filter_add_pattern(filter, s1.c_str());
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
  }
  res = gtk_dialog_run(GTK_DIALOG(dialog));

  if (res == GTK_RESPONSE_ACCEPT) {
    char *filename = gtk_file_chooser_get_filename(chooser);
    filePutContent(filename, getTextViewString(TEXTVIEW_MAIN, false));
    g_free(filename);
  }

  gtk_widget_destroy(dialog);
}

/*
JOB_TYPE_FULL - stop calculations if needed, then start new calculations
JOB_TYPE_SORT_AND_FILTER - stop calculations if needed, then sort and filter
results JOB_TYPE_FILTER - stop calculations if needed, then filter results
JOB_TYPE_STOP - stop calculations if needed
*/
void Frame::job(ENUM_JOB_TYPE e) {
  // prsync(magic_enum::enum_name(m_menuClick), magic_enum::enum_name(e));
  if (oneOf(e, JOB_TYPE_SORT_AND_FILTER, JOB_TYPE_FILTER) && m_result.empty()) {
    return;
  }

  if (e == JOB_TYPE_FULL) {
    SearchResult::out = "";
    m_result.clear();
  }
  clearTagMarks();
  m_begin = clock();
  m_addstatus = "";
  m_filteredWordsCount = 0;
  setLabel(m_searchTagLabel, "");

  if (!prepare()) {
    m_end = clock();
    updateStatus(STATE_ERROR);
    return;
  }

  // --- ИСПРАВЛЕНИЕ ДЕДЛОКА ---
  // Если старый менеджер еще активен, мы сигнализируем ему остановиться
  // и ОТСОЕДИНЯЕМ (detach), чтобы деструктор jthread НЕ блокировал UI-поток!
  if (m_managerThread.joinable()) {
    m_managerThread.request_stop();
    m_managerThread
        .detach(); // Теперь присваивание ниже НЕ вызовет .join() в UI
  }

  m_managerThread = std::jthread([this, e](std::stop_token manager_token) {
    if (e != JOB_TYPE_STOP) {
      g_idle_add_full(G_PRIORITY_HIGH, update_status,
                      GINT_TO_POINTER(STATE_PROCEEDING), NULL);
    }
    // Безопасно завершаем предыдущий рабочий поток
    if (m_thread.joinable()) {
      m_thread.request_stop();
      m_thread.join(); // Этот join происходит в фоне, UI не виснет!
    }

    // Обязательно проверяем токен менеджера ПОСЛЕ того, как дождались старый
    // m_thread
    if (manager_token.stop_requested()) {
      return; // Если прилетел новый job, просто выходим. Новый менеджер сделает
              // остальное.
    }

    if (e == JOB_TYPE_STOP) {
      g_idle_add_full(G_PRIORITY_HIGH, end_job, NULL, NULL);
      return;
    }

    // Запускаем новый рабочий поток
    m_thread = std::jthread([this, e](std::stop_token token) {
      m_token = token;

      // g_idle_add_full(G_PRIORITY_HIGH, update_status,
      //                 GINT_TO_POINTER(STATE_PROCEEDING), NULL);

      if (token.stop_requested())
        return;

      // Запуск основной работы
      run(e);

      // Проверяем, не отменили ли нас пока работал run(e)
      if (token.stop_requested())
        return;

      g_idle_add_full(G_PRIORITY_HIGH, end_job, NULL, NULL);
    });
  });
}

void Frame::windowDeleteEvent() {
  gtk_window_get_position(GTK_WINDOW(m_widget), &m_x, &m_y);
  gtk_window_get_size(GTK_WINDOW(m_widget), &m_width, &m_height);
}

void Frame::addHelp(ENUM_STRING e) {
  auto w = gtk_label_new("");
  gtk_container_add(GTK_CONTAINER(m_helperUp), w);
  gtk_label_set_justify(GTK_LABEL(w), GTK_JUSTIFY_FILL);
  gtk_label_set_line_wrap(GTK_LABEL(w), TRUE);
  gtk_label_set_max_width_chars(GTK_LABEL(w), 40);

  auto s = replaceAll(string(e), "<br>", "\n");
  gchar *p = g_markup_printf_escaped(s.c_str());
  gtk_label_set_markup(GTK_LABEL(w), p);
  g_free(p);
}

void Frame::createCheck(ENUM_STRING e, bool set) {
  m_check = gtk_check_button_new_with_label(string(e).c_str());
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_check), set);
  updateCheckValue();
  g_signal_connect(m_check, "toggled", G_CALLBACK(check_changed), NULL);
}

void Frame::checkChanged() {
  updateCheckValue();
  job();
}

void Frame::updateCheckValue() {
  m_checkValue = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_check));
}

void Frame::radioChanged() {
  updateRadioValue();
  job();
}

void Frame::updateRadioValue() { m_radioValue = getSelectedRadioIndex(); }

int Frame::getSelectedRadioIndex() {
  GSList *group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(m_radio));
  int total_buttons = g_slist_length(group);
  int reverse_index = 0;

  for (GSList *l = group; l != NULL; l = l->next) {
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(l->data))) {
      return (total_buttons - 1) - reverse_index;
    }
    reverse_index++;
  }
  return -1; // Fallback if none are selected
}