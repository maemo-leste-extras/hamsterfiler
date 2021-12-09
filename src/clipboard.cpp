#include "clipboard.h"

Clipboard* Clipboard::instance = NULL;

Clipboard* Clipboard::get()
{
    return instance ? instance : instance = new Clipboard();
}

// Load from persistent storage
Clipboard::Clipboard()
{
    contents_m = QSettings().value("Clipboard/Contents").toStringList();
    QList<QVariant> selection = QSettings().value("Clipboard/Selection").toList();
    int i = 0;

    // Load files and selection
    while (i < selection.size() && i < contents_m.size())
        selection_m.append(selection.at(i++).toBool());

    // Some error correction
    while (i++ < contents_m.size())
        selection_m.append(false);

    // Remove stored state after it is loaded
    QSettings().remove("Clipboard/Contents");
    QSettings().remove("Clipboard/Selection");

    // Get rid of files that no longer exist
    i = 0;
    while (i < contents_m.size()) {
        if (!QFileInfo(contents_m.at(i)).exists()) {
            contents_m.removeAt(i);
            selection_m.removeAt(i);
        } else {
            i++;
        }
    }
}

Clipboard::~Clipboard()
{
    instance = NULL;

    if (QSettings().value("Clipboard/SaveOnExit", DEFAULT_SaveOnExit).toBool())
        save();
}

// Save contents to persistent storage
void Clipboard::save()
{
    QList<QVariant> selection;
    for (int i = 0; i < selection_m.size(); i++)
        // Save a few bytes per entry by storing "0" and "1" instad of "true" and "false"
        selection.append(static_cast<int>(selection_m.at(i)));

    QSettings().setValue("Clipboard/Contents", contents_m);
    QSettings().setValue("Clipboard/Selection", selection);
}

// Raw access to the contents
QStringList Clipboard::contents()
{
    return contents_m;
}

// Raw access to the selection
QList<bool> Clipboard::selection()
{
    return selection_m;
}

// Raturn a list of all selected files
QFileInfoList Clipboard::selectedFiles(bool remove)
{
    QFileInfoList files;

    // Destructive operations should request the files to be removed
    if (remove) {
        // Build a list of selected files while removing them from the list
        for (int i = 0; i < selection_m.size(); ) {
            if (selection_m.at(i)) {
                files.append(QFileInfo(contents_m.at(i)));
                contents_m.removeAt(i);
                selection_m.removeAt(i);
            } else {
                i++;
            }
        }
    } else {
        // Build a list of selected files
        for (int i = 0; i < selection_m.size(); i++)
            if (selection_m.at(i))
                files.append(QFileInfo(contents_m.at(i)));

        // Clear the selection
        if (QSettings().value("Clipboard/QueryingClearsSelection", DEFAULT_QueryingClearsSelection).toBool())
            for (int i = 0; i < selection_m.size(); i++)
                selection_m[i] = false;
    }

    return files;
}

// Remove all entries
void Clipboard::clear()
{
    contents_m.clear();
    selection_m.clear();
}

// Add files, discard duplicates
void Clipboard::add(QFileInfoList files)
{
    // Clear current selection if configured to do so
    if (QSettings().value("Clipboard/AddingClearsSelection", DEFAULT_AddingClearsSelection).toBool())
        for (int i = 0; i < selection_m.size(); i++)
            selection_m[i] = false;

    // Check what should be the initial state of new items
    bool initialSelectionState = QSettings().value("Clipboard/SelectNewItems", DEFAULT_SelectNewItems).toBool();

    // Prepend in reverse order to keep recent files at the top and in the right order at the same time
    for (int i = files.size()-1; i >= 0; i--) {
        QString path = files.at(i).absoluteFilePath();
        int duplicateIndex = contents_m.indexOf(path);

        // Do not allow duplicates
        if (duplicateIndex == -1) {
            contents_m.prepend(path);
            selection_m.prepend(initialSelectionState);
        } else {
            contents_m.move(duplicateIndex, 0);
            selection_m.removeAt(duplicateIndex);
            selection_m.prepend(initialSelectionState);
        }
    }
}

// Select or deselect an item by index
void Clipboard::select(int index, bool select)
{
    if (index < selection_m.size())
        selection_m[index] = select;
}

// Remove an item by index
void Clipboard::remove(int index)
{
    if (index < contents_m.size()) {
        contents_m.removeAt(index);
        selection_m.removeAt(index);
    }
}
