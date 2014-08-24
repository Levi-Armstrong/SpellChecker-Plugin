/**************************************************************************
**
** Copyright (c) 2014 Carel Combrink
**
** This file is part of the SpellChecker Plugin, a Qt Creator plugin.
**
** The SpellChecker Plugin is free software: you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public License as
** published by the Free Software Foundation, either version 3 of the
** License, or (at your option) any later version.
**
** The SpellChecker Plugin is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with the SpellChecker Plugin.  If not, see <http://www.gnu.org/licenses/>.
****************************************************************************/

#include "ProjectMistakesModel.h"

#include <coreplugin/editormanager/editormanager.h>

#include <QFileInfo>

using namespace SpellChecker::Internal;

typedef QMap<QString,SpellChecker::WordList> FileMistakes;

class SpellChecker::Internal::ProjectMistakesModelPrivate {
public:
    FileMistakes spellingMistakes;

    ProjectMistakesModelPrivate() {}
};
//--------------------------------------------------
//--------------------------------------------------
//--------------------------------------------------

ProjectMistakesModel::ProjectMistakesModel() :
    d(new ProjectMistakesModelPrivate())
{
}
//--------------------------------------------------

ProjectMistakesModel::~ProjectMistakesModel()
{
    delete d;
}
//--------------------------------------------------

void ProjectMistakesModel::insertSpellingMistakes(const QString &fileName, const SpellChecker::WordList &words)
{
    /* Check if the model already contains the file */
    FileMistakes::iterator file = d->spellingMistakes.find(fileName);

    if((words.isEmpty() == true)
        && (file == d->spellingMistakes.end())) {
        /* The file was never added, no need to do anything */
        return;
    }

    /* If there are no mistakes in the list of words, remove the file
     * from the model if it was found in the list of files. */
    if((words.isEmpty() == true)
        && (file != d->spellingMistakes.end())) {
        int idx = indexOfFile(fileName);
        Q_ASSERT(idx != -1);
        beginRemoveRows(QModelIndex(), idx, idx);
        d->spellingMistakes.remove(fileName);
        endRemoveRows();
        return;
    }

    /* So there are misspelled words */
    if(file != d->spellingMistakes.end()) {
        /* The file was added with mistakes before, check if there are a change in the
         * total number of items. */
        bool changed = (file.value().count() != words.count());
        /* Assign the words to the file */
        file.value() = words;
        /* nofify of the change if there was one */
        if(changed == true) {
            int idx = indexOfFile(fileName);
            Q_ASSERT(idx != -1);
            emit dataChanged(index(idx, 0, QModelIndex()), index(idx, columnCount(QModelIndex()), QModelIndex()));
        }
    } else {
        /* File was never added... Make a copy of the mistakes and add the
         * new file into the copy to get the index of where the file will
         * get added into the real list. This is then used to notify the
         * model where the insert will occur. Then add the mistakes to the
         * real file. */
        FileMistakes tmpFiles = d->spellingMistakes;
        tmpFiles.insert(fileName, WordList());
        int idx = tmpFiles.keys().indexOf(fileName);
        beginInsertRows(QModelIndex(), idx, idx);
        d->spellingMistakes.insert(fileName, words);
        endInsertRows();
    }
}
//--------------------------------------------------

void ProjectMistakesModel::clearAllSpellingMistakes()
{
    beginResetModel();
    d->spellingMistakes.clear();
    endResetModel();
}
//--------------------------------------------------

void ProjectMistakesModel::fileSelected(const QModelIndex &index)
{
    QString fileName = index.data(COLUMN_FILEPATH).toString();
    if(QFileInfo(fileName).exists() == true) {
        Core::EditorManager::openEditor(fileName);
    }
}
//--------------------------------------------------

QModelIndex ProjectMistakesModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() == true) {
        return QModelIndex();
    } else {
        return createIndex(row, column);
    }
}
//--------------------------------------------------

QModelIndex ProjectMistakesModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child);
    return QModelIndex();
}
//--------------------------------------------------

int ProjectMistakesModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() == true) {
        return 0;
    } else {
        return d->spellingMistakes.count();
    }
}
//--------------------------------------------------

int ProjectMistakesModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid() == true) {
        return 0;
    } else {
        return COLUMN_COUNT - Qt::UserRole;
    }
}
//--------------------------------------------------

QVariant ProjectMistakesModel::data(const QModelIndex &index, int role) const
{
    if ((index.isValid() == false)
            || (index.row() < 0)
            || (index.row() >= rowCount(QModelIndex()))
            || (role <= Qt::DisplayRole)) {
        return QVariant();
    }

    FileMistakes::ConstIterator iter = d->spellingMistakes.constBegin() + index.row();
    if(iter == d->spellingMistakes.constEnd()) {
        return QVariant();
    }

    switch(role) {
    case COLUMN_FILE:
        return QFileInfo(iter.key()).fileName();
    case COLUMN_MISTAKES_TOTAL:
        return (iter.value().count());
    case COLUMN_FILEPATH:
        return iter.key();
    default:
        return QVariant();
    }
}
//--------------------------------------------------

int ProjectMistakesModel::indexOfFile(const QString &fileName) const
{
    return d->spellingMistakes.keys().indexOf(fileName);
}
//--------------------------------------------------
//--------------------------------------------------
//--------------------------------------------------
//--------------------------------------------------
