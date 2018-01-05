#pragma warning(push, 0)
#include <FL/Fl_Native_File_Chooser.H>
#pragma warning(pop)

#include "file-choosers.h"

Open_DTED_Chooser::Open_DTED_Chooser() : Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_FILE) {
	title("Open DTED File");
	filter("PNG File\t*.png\n");
}

Save_DTED_Chooser::Save_DTED_Chooser() : Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_SAVE_FILE) {
	title("Save DTED File");
	filter("PNG File\t*.png\n");
	preset_file("output.png");
}
