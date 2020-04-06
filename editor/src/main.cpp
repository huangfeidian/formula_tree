#include <QApplication>
#include "formula_editor_window.h"

using namespace std;
using namespace spiritsaway::tree_editor;
using namespace spiritsaway::formula_tree::editor;
int main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	formula_editor_window w = formula_editor_window();
	w.showMaximized();
	w.show();
	if (!w.load_config())
	{
		return 0;
	}
	return a.exec();
}