/*
 * Frame.h
 *
 *  Created on: 14.09.2015
 *      Author: alexey slovesnov
 */

#ifndef FRAME_H_
#define FRAME_H_

#include "common/consts.h"
#include "common/WordsBase.h"
#include "CheckNewVersion.h"

typedef std::map<ENUM_MENU,GtkWidget *> MenuMap;

class Frame : WordsBase{
	static const int COMBOLINE_MARGIN=3;

	GtkWidget*m_widget;
	GtkWidget*m_menu;
	GtkWidget*m_text;
	GtkWidget*m_helperUp;
	GtkWidget*m_combo[COMBOBOX_SIZE];
	GtkWidget*m_entry;
	GtkWidget*m_status;
	GtkWidget*m_statusMessage;
	GtkWidget*m_searchLabel;
	GtkWidget*m_searchTagLabel;
	GtkWidget*m_searchButton[2];//next, previous buttons
	GtkWidget*m_searchEntry;
	GtkWidget*m_currentDictionary;
	GtkWidget*m_check;

	MenuMap m_menuMap;
	GtkAccelGroup* m_accelGroup[MENU_ACCEL_SIZE];
	bool m_lockSignals;
	int m_tagIndex;
	int m_tags;
	GThread* m_thread;//uses only in main thread
	GMutex m_mutex;

	/* Note should be a variable
	 * cann't create CheckNewVersion object in Frame::Frame()
	 * because in this case ~CheckNewVersion() is called in Frame::Frame()
	 * and ~CheckNewVersion() is waiting while thread is finished so program slow down
	 */
	CheckNewVersion m_newVersion;

	inline gint getComboIndex(ENUM_COMBOBOX e)const{
		assert(e!=COMBOBOX_SIZE);
		assert(GTK_IS_COMBO_BOX(m_combo[e]));
		return gtk_combo_box_get_active(GTK_COMBO_BOX(m_combo[e]));
	}

	inline void setComboIndex(ENUM_COMBOBOX e,gint v){
		assert(GTK_IS_COMBO_BOX(m_combo[e]));
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_combo[e]),v);
		updateComboValue(e);
	}

	void updateComboValue(ENUM_COMBOBOX e);

	void aboutDialog();
	void setHelperPanel();

	void createImageCombo(ENUM_COMBOBOX e);
	GtkWidget* createTextCombo(ENUM_COMBOBOX e,StringVector v, int active);
	GtkWidget* createTextCombo(ENUM_COMBOBOX e,int from,int to, int active);
	GtkWidget* createTextCombo(ENUM_COMBOBOX e,ENUM_STRING from,ENUM_STRING to, int active);

	static inline void addClass(GtkWidget*w, const gchar* s){
		GtkStyleContext *context;
		context = gtk_widget_get_style_context (w);
		gtk_style_context_add_class (context,s );
	}

	static inline void removeClass(GtkWidget*w, const gchar* s){
		GtkStyleContext *context;
		context = gtk_widget_get_style_context (w);
		gtk_style_context_remove_class (context,s );
	}

	GtkWidget* createEntry(int i);

	void addEntryLineToHelper(int i);
	void addComboLineToHelper(ENUM_STRING id, int from, int to, int active, ENUM_STRING eid);
	void addComboToHelper(ENUM_STRING from,ENUM_STRING to,int active);
	void addComboLineToHelper(ENUM_STRING id, int from, int to, int active, std::string s1, std::string s2, std::string s3);

	void updateTags();

	/*make non static because other createLabel() functions is not static*/
	inline GtkWidget* createLabel(std::string s){
		return gtk_label_new(s.c_str());
	}

	inline GtkWidget* createLabel(ENUM_STRING e){
		return createLabel(m_language[e]);
	}

	/*make non static because other add() functions is not static*/
	inline void add(GtkWidget*w,GtkWidget*a){
		gtk_box_pack_start (GTK_BOX(w), a, TRUE, TRUE, 0);
	}

	inline void add(GtkWidget*w,std::string s){
		add(w,createLabel(s));
	}

	inline void add(GtkWidget*w,ENUM_STRING e){
		add(w,createLabel(e));
	}

	void clearHelper();

public:
	Frame();
	virtual ~Frame(){
		g_mutex_clear(&m_mutex);
	};
	void destroy();

	void startJob(bool clearResult);
	void endJob();
	bool prepare();//return true if entry data is valid

	void openURL(std::string url);
	void routine();
	void sortAndUpdateResults();

	inline bool isSignalsLocked(){
		return m_lockSignals;
	}

	inline void lockSignals(){
		m_lockSignals=true;
	}

	inline void unlockSignals(){
		m_lockSignals=false;
	}

	std::string getImagePath(std::string name){
		return getResourcePath("images/"+name);
	}

	void clickMenu(ENUM_MENU menu);

	void setDictionary();
	void loadAndUpdateCurrentLanguage();
	void comboChanged(ENUM_COMBOBOX e);
	void entryChanged(int entryIndex);
	void clickButton(GtkWidget* button);
	void checkChanged(GtkWidget* check){
		stopThreadAndNewRoutine();
	}

	void stopThreadAndNewRoutine(){
		stopThread();
		routine();
	}

	virtual void setMenuLabel(ENUM_MENU e,std::string const& text);
	std::string getMenuLabel(ENUM_MENU e);

	void proceedThread();
	void stopThread();
	virtual bool userBreakThread();
	void waitThread();
	inline void startThread(GThreadFunc f){
		m_thread=g_thread_new("",f,NULL);
	}

	void updateTextView(){
		GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (m_text));
		gtk_text_buffer_set_text (buffer, m_out.c_str(), -1);
	}

	void setStatus(std::string const& s){
		gtk_label_set_text(GTK_LABEL(m_statusMessage),s.c_str());
	}

	void connectEntrySignals(int index);
	void entryFocusChanged(bool in);
	void removeAccelerators();
	void addAccelerators();

};

#endif /* FRAME_H_ */
