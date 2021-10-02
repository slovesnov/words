/*
 * CheckNewVersion.cpp
 *
 *  Created on: 14.02.2018
 *      Author: alexey slovesnov
 */

#include <stdlib.h> //atof
#include "CheckNewVersion.h"
#include "Frame.h"

extern Frame* frame;
static CheckNewVersion*pCheckNewVersion;

StringVector split(const std::string& subject, const std::string& separator) {
	StringVector r;
	size_t pos,prev;
	for( prev=0 ; (pos = subject.find(separator, prev)) != std::string::npos ; prev= pos+separator.length() ) {
		r.push_back(subject.substr(prev,pos-prev));
	}
	r.push_back(subject.substr(prev,subject.length()));
	return r;
}

static gpointer check_new_nersion_thread(gpointer){
	pCheckNewVersion->routine();
	return NULL;
}

static gboolean new_version_message (gpointer){
	pCheckNewVersion->newVesionMessage();
	return G_SOURCE_REMOVE;
}

CheckNewVersion::CheckNewVersion() {
	//to avoid warnings
	m_parent=0;
//	m_url=url;
	m_version=0;
//	m_message=message;
  m_newVersionThread=0;
}

void CheckNewVersion::init(GtkWindow *parent, std::string title, std::string versionUrl, std::string downloadUrl, double version,
		std::string message) {
	pCheckNewVersion=this;
	m_parent=parent;
	m_title=title;
	m_versionUrl=versionUrl;
	m_downloadUrl=downloadUrl;
	m_version=version;
	m_message=message;
  m_newVersionThread=g_thread_new("",check_new_nersion_thread,NULL);
}

void CheckNewVersion::routine() {
	const char*uri=m_versionUrl.c_str();

	GFile * f=g_file_new_for_uri (uri);//f is always not null
	GError *error=NULL;
	gsize length;
	char* content=NULL;
	gboolean r=g_file_load_contents (f,NULL,&content,&length,NULL,&error);

	if(r){
		std::string s(content,length);//content is not null terminated so create std::string
		//g_print("[%s]\n",s.c_str());
		StringVector vs=split(s,"\n");

	  //set dot as decimal separator, standard locale
	 	setlocale(LC_NUMERIC, "C");//dot interpret as decimal separator

		if( atof(vs[0].c_str()) > m_version ){
			StringVectorCI it;
			for(it=vs.begin();it!=vs.end();it++){
				if(it!=vs.begin()){
					m_message+=WordsBase::localeToUtf8("\n"+*it);
				}
			}
			gdk_threads_add_idle(new_version_message,NULL);
		}
	}
	g_free(content);
	g_object_unref(f);
}

CheckNewVersion::~CheckNewVersion() {
	g_thread_join(m_newVersionThread);
}

void CheckNewVersion::newVesionMessage() {
	GtkWidget *dialog = gtk_message_dialog_new (m_parent,
		GTK_DIALOG_MODAL,
		GTK_MESSAGE_INFO,
		GTK_BUTTONS_YES_NO,
		m_message.c_str());

  gtk_window_set_title(GTK_WINDOW(dialog),m_title.c_str());
  gint result = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy (dialog);

	if(result==GTK_RESPONSE_YES){
		frame->openURL(m_downloadUrl);
	}
}
