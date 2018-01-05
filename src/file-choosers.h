#pragma once

#pragma warning(push, 0)
#include <FL/Fl_Native_File_Chooser.H>
#pragma warning(pop)

class Open_DTED_Chooser : public Fl_Native_File_Chooser {
public:
	Open_DTED_Chooser();
};

class Save_DTED_Chooser : public Fl_Native_File_Chooser {
public:
	Save_DTED_Chooser();
};
