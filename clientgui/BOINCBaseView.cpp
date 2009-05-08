// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2009 David Barnard, Peter Kortschack
// Copyright (C) 2005 University of California
//
// Synecdoche is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Synecdoche is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License with Synecdoche.  If not, see <http://www.gnu.org/licenses/>.

#include "BOINCBaseView.h"
#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "BOINCTaskCtrl.h"
#include "BOINCListCtrl.h"
#include "Events.h"

IMPLEMENT_DYNAMIC_CLASS(CBOINCBaseView, wxPanel)

CBOINCBaseView::CBOINCBaseView() {}

CBOINCBaseView::CBOINCBaseView(wxNotebook* pNotebook) : wxPanel(pNotebook, -1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL) {
    wxASSERT(pNotebook);

    m_bProcessingTaskRenderEvent = false;
    m_bProcessingListRenderEvent = false;

    m_bForceUpdateSelection = true;
    m_bIgnoreUIEvents = false;

    m_bViewLoaded = false;

    m_pListPane = NULL;
    m_iProgressColumn = -1;
    m_iSortColumn = -1;
    
    SetName(GetViewName());

    SetAutoLayout(true);
}

CBOINCBaseView::~CBOINCBaseView() {
}

/// Return the name of the view.
/// If it has not been defined by the view "Undefined" is returned.
///
/// \return A reference to a variable containing the view name.
const wxString& CBOINCBaseView::GetViewName() {
    static wxString strViewName(wxT("Undefined"));
    return strViewName;
}

/// Return the user friendly name of the view.
/// If it has not been defined by the view "Undefined" is returned.
///
/// \return A reference to a variable containing the user friendly name of
///         the view.
const wxString& CBOINCBaseView::GetViewDisplayName() {
    static wxString strViewName(wxT("Undefined"));
    return strViewName;
}

/// Return the user friendly icon of the view.
///
/// \return Always returns a null-pointer.
const char** CBOINCBaseView::GetViewIcon() {
    return 0;
}

/// The rate at which the view is refreshed.
/// If it has not been defined by the view 1 second is retrned.
///
/// \return Always returns one.
const int CBOINCBaseView::GetViewRefreshRate() {
    return 1;
}

bool CBOINCBaseView::FireOnSaveState(wxConfigBase* pConfig) {
    if (m_bViewLoaded) return OnSaveState(pConfig);
    return true;
}

bool CBOINCBaseView::FireOnRestoreState(wxConfigBase* pConfig) {
    if (m_bViewLoaded) return OnRestoreState(pConfig);
    // What does the return value mean?
    return true;
}

int CBOINCBaseView::GetListRowCount() {
    wxASSERT(m_pListPane);
    return m_pListPane->GetItemCount();
}

void CBOINCBaseView::FireOnListRender(wxTimerEvent& event) {
    if (m_bViewLoaded) OnListRender(event);
}

void CBOINCBaseView::FireOnListSelected(wxListEvent& event) {
    OnListSelected(event);
}

wxString CBOINCBaseView::FireOnListGetItemText(long item, long column) const {
    return OnListGetItemText(item, column);
}

int CBOINCBaseView::FireOnListGetItemImage(long item) const {
    return OnListGetItemImage(item);
}

wxListItemAttr* CBOINCBaseView::FireOnListGetItemAttr(long item) const {
    return OnListGetItemAttr(item);
}

void CBOINCBaseView::FireOnShowView() {
    if (!m_bViewLoaded) {
        Freeze();
        DemandLoadView();
        Thaw();
        m_bViewLoaded = true;
    }
}

void CBOINCBaseView::OnListRender(wxTimerEvent& event) {
    if (!m_bProcessingListRenderEvent) {
        m_bProcessingListRenderEvent = true;

        wxASSERT(m_pListPane);

        int iDocCount = GetDocCount();
        int iCacheCount = GetCacheCount();
        if (iDocCount != iCacheCount) {
            if (0 >= iDocCount) {
                m_pListPane->DeleteAllItems();
                EmptyCache();
            } else {
                int iIndex = 0;
                int iReturnValue = -1;
                if (iDocCount > iCacheCount) {
                    for (iIndex = 0; iIndex < (iDocCount - iCacheCount); ++iIndex) {
                        iReturnValue = AddCacheElement();
                        wxASSERT(!iReturnValue);
                    }
                    wxASSERT(GetDocCount() == GetCacheCount());
                    m_pListPane->SetItemCount(iDocCount);
                } else {
                    // We can't just call SetItemCount() here because we need to
                    // let the virtual ListCtrl adjust its list of selected rows
                    // to remove (deselect) any beyond the new last row
                    for (iIndex = (iCacheCount - 1); iIndex >= iDocCount; --iIndex) {
                        m_pListPane->DeleteItem(iIndex);
                        iReturnValue = RemoveCacheElement();
                        wxASSERT(!iReturnValue);
                    }
                    wxASSERT(GetDocCount() == GetCacheCount());
                    m_pListPane->RefreshItems(0, iDocCount - 1);
                }
            }
        }

        if (iDocCount > 0) {
            SynchronizeCache();


            if (EnsureLastItemVisible() && (iDocCount != iCacheCount)) {
                m_pListPane->EnsureVisible(iDocCount - 1);
            }

            if (m_pListPane->m_bIsSingleSelection) {
                // If no item has been selected yet, select the first item.
#ifdef __WXMSW__
                if ((m_pListPane->GetSelectedItemCount() == 0) &&
                    (m_pListPane->GetItemCount() >= 1)) {

                    long desiredstate = wxLIST_STATE_FOCUSED | wxLIST_STATE_SELECTED;
                    m_pListPane->SetItemState(0, desiredstate, desiredstate);
                }
#else
                if ((m_pListPane->GetFirstSelected() < 0) &&
                    (m_pListPane->GetItemCount() >= 1)) {
                    m_pListPane->SetItemState(0, wxLIST_STATE_FOCUSED | wxLIST_STATE_SELECTED, 
                                                    wxLIST_STATE_FOCUSED | wxLIST_STATE_SELECTED);
                }
#endif
            }
        }

        UpdateSelection();

        m_bProcessingListRenderEvent = false;
    }

    event.Skip();
}

bool CBOINCBaseView::OnSaveState(wxConfigBase* pConfig) {
    wxASSERT(pConfig);
    wxASSERT(m_pListPane);

    return m_pListPane->OnSaveState(pConfig);
}

bool CBOINCBaseView::OnRestoreState(wxConfigBase* pConfig) {
    wxASSERT(pConfig);
    wxASSERT(m_pListPane);

    return m_pListPane->OnRestoreState(pConfig);
}

void CBOINCBaseView::OnListSelected(wxListEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseView::OnListSelected - Function Begin"));

    if (!m_bIgnoreUIEvents) {
        m_bForceUpdateSelection = true;
        UpdateSelection();
        event.Skip();
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseView::OnListSelected - Function End"));
}

// Work around a bug (feature?) in virtual list control 
//   which does not send deselection events
void CBOINCBaseView::OnCacheHint(wxListEvent& event) {
    static int oldSelectionCount = 0;
    int newSelectionCount = m_pListPane->GetSelectedItemCount();

    if (newSelectionCount < oldSelectionCount) {
        wxListEvent leDeselectedEvent(wxEVT_COMMAND_LIST_ITEM_DESELECTED, m_windowId);
        leDeselectedEvent.SetEventObject(this);
        OnListSelected(leDeselectedEvent);
    }
    oldSelectionCount = newSelectionCount;
    event.Skip();
}

wxString CBOINCBaseView::OnListGetItemText(long WXUNUSED(item), long WXUNUSED(column)) const {
    return wxString(wxT("Undefined"));
}

int CBOINCBaseView::OnListGetItemImage(long WXUNUSED(item)) const {
    return -1;
}

wxListItemAttr* CBOINCBaseView::OnListGetItemAttr(long WXUNUSED(item)) const {
    return NULL;
}

void CBOINCBaseView::OnGridSelectCell( wxGridEvent& event ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseView::OnGridSelectCell - Function Begin"));

    if (!m_bIgnoreUIEvents) {
        m_bForceUpdateSelection = true;
        UpdateSelection();
        event.Skip();
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseView::OnGridSelectCell - Function End"));
}

void CBOINCBaseView::OnGridSelectRange( wxGridRangeSelectEvent& event ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseView::OnGridSelectRange - Function Begin"));

    if (!m_bIgnoreUIEvents) {
        m_bForceUpdateSelection = true;
        UpdateSelection();
        event.Skip();
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCBaseView::OnGridSelectRange - Function End"));
}

int CBOINCBaseView::GetDocCount() {
    return 0;
}

wxString CBOINCBaseView::OnDocGetItemImage(long WXUNUSED(item)) const {
    return wxString(wxT("Undefined"));
}

wxString CBOINCBaseView::OnDocGetItemAttr(long WXUNUSED(item)) const {
    return wxString(wxT("Undefined"));
}

int CBOINCBaseView::AddCacheElement() {
    return -1;
}
    
int CBOINCBaseView::EmptyCache() {
    return -1;
}

int CBOINCBaseView::GetCacheCount() {
    return -1;
}

int CBOINCBaseView::RemoveCacheElement() {
    return -1;
}

int CBOINCBaseView::SynchronizeCache() {
    int         iRowIndex        = 0;
    int         iRowTotal        = 0;
    int         iColumnIndex     = 0;
    int         iColumnTotal     = 0;
    bool        bNeedRefreshData = false;
    bool        bNeedSort = false;

    iRowTotal = GetDocCount();
    iColumnTotal = m_pListPane->GetColumnCount();

    for (iRowIndex = 0; iRowIndex < iRowTotal; iRowIndex++) {
        bNeedRefreshData = false;

        for (iColumnIndex = 0; iColumnIndex < iColumnTotal; iColumnIndex++) {
            if (SynchronizeCacheItem(iRowIndex, iColumnIndex)) {
                bNeedRefreshData = true;
                if (iColumnIndex == m_iSortColumn) {
                    bNeedSort = true;
                }
            }
        }

        if (bNeedRefreshData) {
            m_pListPane->RefreshItem(iRowIndex);
        }
    }

    if (bNeedSort) {
        sortData();     // Will mark entire list as needing refresh
    }
    return 0;
}

bool CBOINCBaseView::SynchronizeCacheItem(wxInt32 WXUNUSED(iRowIndex), wxInt32 WXUNUSED(iColumnIndex)) {
    return false;
}

void CBOINCBaseView::OnColClick(wxListEvent& event) {
    wxListItem      item;
    int             newSortColumn = event.GetColumn();

    item.SetMask(wxLIST_MASK_IMAGE);
    if (newSortColumn == m_iSortColumn) {
        m_bReverseSort = !m_bReverseSort;
    } else {
        // Remove sort arrow from old sort column
        if (m_iSortColumn >= 0) {
            item.SetImage(-1);
            m_pListPane->SetColumn(m_iSortColumn, item);
        }
        m_iSortColumn = newSortColumn;
        m_bReverseSort = false;
    }
    
    item.SetImage(m_bReverseSort ? 0 : 1);
    m_pListPane->SetColumn(newSortColumn, item);
    sortData();
}

void CBOINCBaseView::InitSort() {
    wxListItem      item;

    if (m_iSortColumn < 0) return;
    item.SetMask(wxLIST_MASK_IMAGE);
    item.SetImage(m_bReverseSort ? 0 : 1);
    m_pListPane->SetColumn(m_iSortColumn, item);
    sortData();
}

void CBOINCBaseView::sortData() {
    if (m_iSortColumn < 0) return;
    
    std::vector<size_t> oldSortedIndexes(m_iSortedIndexes);
    std::vector<size_t> selections;
    size_t n = m_iSortedIndexes.size();
    
    // Remember which cache elements are selected and deselect them
    m_bIgnoreUIEvents = true;
    int i = -1;
    while (1) {
        i = m_pListPane->GetNextItem(i, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (i < 0) break;
        selections.push_back(m_iSortedIndexes.at(i));
        m_pListPane->SetItemState(i, 0, wxLIST_STATE_SELECTED);
    }
    
    std::stable_sort(m_iSortedIndexes.begin(), m_iSortedIndexes.end(), m_funcSortCompare);
    
    // Reselect previously selected cache elements in the sorted list 
    size_t m = selections.size();
    for (size_t i = 0; i < m; ++i) {
        if (selections[i] >= 0) {
            size_t j = m_iSortedIndexes.at(selections[i]);
            m_pListPane->SetItemState(static_cast<long>(j), wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        }
    }
    m_bIgnoreUIEvents = false;

    // Refresh rows which have moved
    for (size_t i = 0; i < n; ++i) {
        if (m_iSortedIndexes[i] != oldSortedIndexes[i]) {
            m_pListPane->RefreshItem(static_cast<long>(i));
         }
    }
}

void CBOINCBaseView::PreUpdateSelection(){
}

void CBOINCBaseView::UpdateSelection(){
}

void CBOINCBaseView::PostUpdateSelection() {
    Layout();
}

bool CBOINCBaseView::_EnsureLastItemVisible() {
    return EnsureLastItemVisible();
}

bool CBOINCBaseView::EnsureLastItemVisible() {
    return false;
}

double CBOINCBaseView::GetProgressValue(long) {
    return 0.0;
}

void CBOINCBaseView::AppendToStatus(wxString& existing, const wxChar* additional) {
    if (existing.IsEmpty()) {
        existing = additional;
    } else {
        existing = existing + wxT(", ") + additional;
    }
}

/// HTML Entity Encoding.
/// See http://www.webreference.com/html/reference/character/
/// Completed: The ISO Latin 1 Character Set
///
/// \param[in] strRaw The string that should be encoded.
/// \return The encoded version of \a strRaw
wxString CBOINCBaseView::HtmlEntityEncode(const wxString& strRaw) {
    wxString strEncodedHtml(strRaw);

#ifdef __WXMSW__
    strEncodedHtml.Replace(wxT("&"),  wxT("&amp;"),    true);
    strEncodedHtml.Replace(wxT("\""), wxT("&quot;"),   true);
    strEncodedHtml.Replace(wxT("<"),  wxT("&lt;"),     true);
    strEncodedHtml.Replace(wxT(">"),  wxT("&gt;"),     true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&sbquo;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&fnof;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&bdquo;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&hellip;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&dagger;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Dagger;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Scaron;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&OElig;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&lsquo;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&rsquo;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&ldquo;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&rdquo;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&bull;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&ndash;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&mdash;"),  true);
    strEncodedHtml.Replace(wxT("��~"),  wxT("&tilde;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&trade;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&scaron;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&oelig;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Yuml;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&iexcl;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&cent;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&pound;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&curren;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&yen;"),    true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&brvbar;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&sect;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&uml;"),    true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&copy;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&ordf;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&laquo;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&not;"),    true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&reg;"),    true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&macr;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&deg;"),    true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&plusmn;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&sup2;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&sup3;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&acute;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&micro;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&para;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&middot;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&cedil;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&sup1;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&ordm;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&raquo;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&frac14;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&frac12;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&frac34;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&iquest;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Agrave;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Aacute;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Acirc;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Atilde;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Auml;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Aring;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&AElig;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Ccedil;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Egrave;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Eacute;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Ecirc;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Euml;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Igrave;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Iacute;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Icirc;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Iuml;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&ETH;"),    true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Ntilde;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Ograve;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Oacute;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Ocirc;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Otilde;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Ouml;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&times;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Oslash;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Ugrave;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Uacute;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Ucirc;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Uuml;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&Yacute;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&THORN;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&szlig;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&agrave;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&aacute;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&acirc;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&atilde;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&auml;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&aring;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&aelig;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&ccedil;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&egrave;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&eacute;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&ecirc;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&euml;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&igrave;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&iacute;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&icirc;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&iuml;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&eth;"),    true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&ntilde;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&ograve;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&oacute;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&ocirc;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&otilde;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&ouml;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&divide;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&oslash;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&ugrave;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&uacute;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&ucirc;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&uuml;"),   true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&yacute;"), true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&thorn;"),  true);
    strEncodedHtml.Replace(wxT("�"),  wxT("&yuml;"),   true);
#endif
    return strEncodedHtml;
}

/// HTML Entity Decoding.
/// See http://www.webreference.com/html/reference/character/
/// Completed: The ISO Latin 1 Character Set
///
/// \param[in] strRaw The string that should be decoded.
/// \return The decoded version of \a strRaw
wxString CBOINCBaseView::HtmlEntityDecode(const wxString& strRaw) {
    wxString strDecodedHtml(strRaw);

    if (0 <= strDecodedHtml.Find(wxT("&"))) {
#ifdef __WXMSW__
        strDecodedHtml.Replace(wxT("&amp;"),    wxT("&"),  true);
        strDecodedHtml.Replace(wxT("&quot;"),   wxT("\""), true);
        strDecodedHtml.Replace(wxT("&lt;"),     wxT("<"),  true);
        strDecodedHtml.Replace(wxT("&gt;"),     wxT(">"),  true);
        strDecodedHtml.Replace(wxT("&sbquo;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&fnof;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&bdquo;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&hellip;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&dagger;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Dagger;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Scaron;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&OElig;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&lsquo;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&rsquo;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&ldquo;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&rdquo;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&bull;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&ndash;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&mdash;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&tilde;"),  wxT("��~"),  true);
        strDecodedHtml.Replace(wxT("&trade;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&scaron;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&oelig;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Yuml;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&iexcl;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&cent;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&pound;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&curren;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&yen;"),    wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&brvbar;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&sect;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&uml;"),    wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&copy;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&ordf;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&laquo;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&not;"),    wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&reg;"),    wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&macr;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&deg;"),    wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&plusmn;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&sup2;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&sup3;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&acute;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&micro;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&para;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&middot;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&cedil;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&sup1;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&ordm;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&raquo;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&frac14;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&frac12;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&frac34;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&iquest;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Agrave;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Aacute;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Acirc;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Atilde;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Auml;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Aring;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&AElig;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Ccedil;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Egrave;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Eacute;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Ecirc;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Euml;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Igrave;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Iacute;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Icirc;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Iuml;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&ETH;"),    wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Ntilde;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Ograve;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Oacute;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Ocirc;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Otilde;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Ouml;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&times;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Oslash;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Ugrave;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Uacute;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Ucirc;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Uuml;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&Yacute;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&THORN;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&szlig;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&agrave;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&aacute;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&acirc;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&atilde;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&auml;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&aring;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&aelig;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&ccedil;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&egrave;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&eacute;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&ecirc;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&euml;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&igrave;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&iacute;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&icirc;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&iuml;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&eth;"),    wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&ntilde;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&ograve;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&oacute;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&ocirc;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&otilde;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&ouml;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&divide;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&oslash;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&ugrave;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&uacute;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&ucirc;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&uuml;"),   wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&yacute;"), wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&thorn;"),  wxT("�"),  true);
        strDecodedHtml.Replace(wxT("&yuml;"),   wxT("�"),  true);
#endif
    }
    return strDecodedHtml;
}

/// Add a new column to the tab.
///
/// \param[in] column The index of the new column.
/// \param[in] heading The title of the column. This parameter is used as key
///                    to identify the column when storing settings. Therefore
///                    this parameter must not be translated. This function
///                    handles translation of the title itself before passing
///                    it to wxWidgets.
/// \param[in] align Column alignment.
/// \param[in] width The width of the column.
void CBOINCBaseView::AddColumn(long column, const wxChar* heading,
                               wxListColumnFormat align, int width) {
    m_pListPane->InsertColumn(column, wxGetTranslation(heading), align, width);
    m_column_keys[column] = heading;
}

/// Get a map containing the keys for all columns.
///
/// \return A map containing the keys used to identify the columns when storing
///         column related settings. This map is filled when AddColumn is
///         called.
const ColumnListMap& CBOINCBaseView::GetColumnKeys() const {
    return m_column_keys;
}

/// Read and apply stored settings like column widths.
void CBOINCBaseView::RestoreState() {
    wxConfigBase* pConfig = wxConfigBase::Get(FALSE);
    wxString strPreviousLocation = pConfig->GetPath();
    wxString strConfigLocation = strPreviousLocation + GetViewName();

    pConfig->SetPath(strConfigLocation);
    OnRestoreState(pConfig);
    pConfig->SetPath(strPreviousLocation);
}
