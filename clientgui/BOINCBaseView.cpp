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
    strEncodedHtml.Replace(wxT("‚"),  wxT("&sbquo;"),  true);
    strEncodedHtml.Replace(wxT("ƒ"),  wxT("&fnof;"),   true);
    strEncodedHtml.Replace(wxT("„"),  wxT("&bdquo;"),  true);
    strEncodedHtml.Replace(wxT("…"),  wxT("&hellip;"), true);
    strEncodedHtml.Replace(wxT("†"),  wxT("&dagger;"), true);
    strEncodedHtml.Replace(wxT("‡"),  wxT("&Dagger;"), true);
    strEncodedHtml.Replace(wxT("Š"),  wxT("&Scaron;"), true);
    strEncodedHtml.Replace(wxT("Œ"),  wxT("&OElig;"),  true);
    strEncodedHtml.Replace(wxT("‘"),  wxT("&lsquo;"),  true);
    strEncodedHtml.Replace(wxT("’"),  wxT("&rsquo;"),  true);
    strEncodedHtml.Replace(wxT("“"),  wxT("&ldquo;"),  true);
    strEncodedHtml.Replace(wxT("”"),  wxT("&rdquo;"),  true);
    strEncodedHtml.Replace(wxT("•"),  wxT("&bull;"),   true);
    strEncodedHtml.Replace(wxT("–"),  wxT("&ndash;"),  true);
    strEncodedHtml.Replace(wxT("—"),  wxT("&mdash;"),  true);
    strEncodedHtml.Replace(wxT("˜˜~"),  wxT("&tilde;"),  true);
    strEncodedHtml.Replace(wxT("™"),  wxT("&trade;"),  true);
    strEncodedHtml.Replace(wxT("š"),  wxT("&scaron;"), true);
    strEncodedHtml.Replace(wxT("œ"),  wxT("&oelig;"),  true);
    strEncodedHtml.Replace(wxT("Ÿ"),  wxT("&Yuml;"),   true);
    strEncodedHtml.Replace(wxT("¡"),  wxT("&iexcl;"),  true);
    strEncodedHtml.Replace(wxT("¢"),  wxT("&cent;"),   true);
    strEncodedHtml.Replace(wxT("£"),  wxT("&pound;"),  true);
    strEncodedHtml.Replace(wxT("¤"),  wxT("&curren;"), true);
    strEncodedHtml.Replace(wxT("¥"),  wxT("&yen;"),    true);
    strEncodedHtml.Replace(wxT("¦"),  wxT("&brvbar;"), true);
    strEncodedHtml.Replace(wxT("§"),  wxT("&sect;"),   true);
    strEncodedHtml.Replace(wxT("¨"),  wxT("&uml;"),    true);
    strEncodedHtml.Replace(wxT("©"),  wxT("&copy;"),   true);
    strEncodedHtml.Replace(wxT("ª"),  wxT("&ordf;"),   true);
    strEncodedHtml.Replace(wxT("«"),  wxT("&laquo;"),  true);
    strEncodedHtml.Replace(wxT("¬"),  wxT("&not;"),    true);
    strEncodedHtml.Replace(wxT("®"),  wxT("&reg;"),    true);
    strEncodedHtml.Replace(wxT("¯"),  wxT("&macr;"),   true);
    strEncodedHtml.Replace(wxT("°"),  wxT("&deg;"),    true);
    strEncodedHtml.Replace(wxT("±"),  wxT("&plusmn;"), true);
    strEncodedHtml.Replace(wxT("²"),  wxT("&sup2;"),   true);
    strEncodedHtml.Replace(wxT("³"),  wxT("&sup3;"),   true);
    strEncodedHtml.Replace(wxT("´"),  wxT("&acute;"),  true);
    strEncodedHtml.Replace(wxT("µ"),  wxT("&micro;"),  true);
    strEncodedHtml.Replace(wxT("¶"),  wxT("&para;"),   true);
    strEncodedHtml.Replace(wxT("·"),  wxT("&middot;"), true);
    strEncodedHtml.Replace(wxT("¸"),  wxT("&cedil;"),  true);
    strEncodedHtml.Replace(wxT("¹"),  wxT("&sup1;"),   true);
    strEncodedHtml.Replace(wxT("º"),  wxT("&ordm;"),   true);
    strEncodedHtml.Replace(wxT("»"),  wxT("&raquo;"),  true);
    strEncodedHtml.Replace(wxT("¼"),  wxT("&frac14;"), true);
    strEncodedHtml.Replace(wxT("½"),  wxT("&frac12;"), true);
    strEncodedHtml.Replace(wxT("¾"),  wxT("&frac34;"), true);
    strEncodedHtml.Replace(wxT("¿"),  wxT("&iquest;"), true);
    strEncodedHtml.Replace(wxT("À"),  wxT("&Agrave;"), true);
    strEncodedHtml.Replace(wxT("Á"),  wxT("&Aacute;"), true);
    strEncodedHtml.Replace(wxT("Â"),  wxT("&Acirc;"),  true);
    strEncodedHtml.Replace(wxT("Ã"),  wxT("&Atilde;"), true);
    strEncodedHtml.Replace(wxT("Ä"),  wxT("&Auml;"),   true);
    strEncodedHtml.Replace(wxT("Å"),  wxT("&Aring;"),  true);
    strEncodedHtml.Replace(wxT("Æ"),  wxT("&AElig;"),  true);
    strEncodedHtml.Replace(wxT("Ç"),  wxT("&Ccedil;"), true);
    strEncodedHtml.Replace(wxT("È"),  wxT("&Egrave;"), true);
    strEncodedHtml.Replace(wxT("É"),  wxT("&Eacute;"), true);
    strEncodedHtml.Replace(wxT("Ê"),  wxT("&Ecirc;"),  true);
    strEncodedHtml.Replace(wxT("Ë"),  wxT("&Euml;"),   true);
    strEncodedHtml.Replace(wxT("Ì"),  wxT("&Igrave;"), true);
    strEncodedHtml.Replace(wxT("Í"),  wxT("&Iacute;"), true);
    strEncodedHtml.Replace(wxT("Î"),  wxT("&Icirc;"),  true);
    strEncodedHtml.Replace(wxT("Ï"),  wxT("&Iuml;"),   true);
    strEncodedHtml.Replace(wxT("Ð"),  wxT("&ETH;"),    true);
    strEncodedHtml.Replace(wxT("Ñ"),  wxT("&Ntilde;"), true);
    strEncodedHtml.Replace(wxT("Ò"),  wxT("&Ograve;"), true);
    strEncodedHtml.Replace(wxT("Ó"),  wxT("&Oacute;"), true);
    strEncodedHtml.Replace(wxT("Ô"),  wxT("&Ocirc;"),  true);
    strEncodedHtml.Replace(wxT("Õ"),  wxT("&Otilde;"), true);
    strEncodedHtml.Replace(wxT("Ö"),  wxT("&Ouml;"),   true);
    strEncodedHtml.Replace(wxT("×"),  wxT("&times;"),  true);
    strEncodedHtml.Replace(wxT("Ø"),  wxT("&Oslash;"), true);
    strEncodedHtml.Replace(wxT("Ù"),  wxT("&Ugrave;"), true);
    strEncodedHtml.Replace(wxT("Ú"),  wxT("&Uacute;"), true);
    strEncodedHtml.Replace(wxT("Û"),  wxT("&Ucirc;"),  true);
    strEncodedHtml.Replace(wxT("Ü"),  wxT("&Uuml;"),   true);
    strEncodedHtml.Replace(wxT("Ý"),  wxT("&Yacute;"), true);
    strEncodedHtml.Replace(wxT("Þ"),  wxT("&THORN;"),  true);
    strEncodedHtml.Replace(wxT("ß"),  wxT("&szlig;"),  true);
    strEncodedHtml.Replace(wxT("à"),  wxT("&agrave;"), true);
    strEncodedHtml.Replace(wxT("á"),  wxT("&aacute;"), true);
    strEncodedHtml.Replace(wxT("â"),  wxT("&acirc;"),  true);
    strEncodedHtml.Replace(wxT("ã"),  wxT("&atilde;"), true);
    strEncodedHtml.Replace(wxT("ä"),  wxT("&auml;"),   true);
    strEncodedHtml.Replace(wxT("å"),  wxT("&aring;"),  true);
    strEncodedHtml.Replace(wxT("æ"),  wxT("&aelig;"),  true);
    strEncodedHtml.Replace(wxT("ç"),  wxT("&ccedil;"), true);
    strEncodedHtml.Replace(wxT("è"),  wxT("&egrave;"), true);
    strEncodedHtml.Replace(wxT("é"),  wxT("&eacute;"), true);
    strEncodedHtml.Replace(wxT("ê"),  wxT("&ecirc;"),  true);
    strEncodedHtml.Replace(wxT("ë"),  wxT("&euml;"),   true);
    strEncodedHtml.Replace(wxT("ì"),  wxT("&igrave;"), true);
    strEncodedHtml.Replace(wxT("í"),  wxT("&iacute;"), true);
    strEncodedHtml.Replace(wxT("î"),  wxT("&icirc;"),  true);
    strEncodedHtml.Replace(wxT("ï"),  wxT("&iuml;"),   true);
    strEncodedHtml.Replace(wxT("ð"),  wxT("&eth;"),    true);
    strEncodedHtml.Replace(wxT("ñ"),  wxT("&ntilde;"), true);
    strEncodedHtml.Replace(wxT("ò"),  wxT("&ograve;"), true);
    strEncodedHtml.Replace(wxT("ó"),  wxT("&oacute;"), true);
    strEncodedHtml.Replace(wxT("ô"),  wxT("&ocirc;"),  true);
    strEncodedHtml.Replace(wxT("õ"),  wxT("&otilde;"), true);
    strEncodedHtml.Replace(wxT("ö"),  wxT("&ouml;"),   true);
    strEncodedHtml.Replace(wxT("÷"),  wxT("&divide;"), true);
    strEncodedHtml.Replace(wxT("ø"),  wxT("&oslash;"), true);
    strEncodedHtml.Replace(wxT("ù"),  wxT("&ugrave;"), true);
    strEncodedHtml.Replace(wxT("ú"),  wxT("&uacute;"), true);
    strEncodedHtml.Replace(wxT("û"),  wxT("&ucirc;"),  true);
    strEncodedHtml.Replace(wxT("ü"),  wxT("&uuml;"),   true);
    strEncodedHtml.Replace(wxT("ý"),  wxT("&yacute;"), true);
    strEncodedHtml.Replace(wxT("þ"),  wxT("&thorn;"),  true);
    strEncodedHtml.Replace(wxT("ÿ"),  wxT("&yuml;"),   true);
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
        strDecodedHtml.Replace(wxT("&sbquo;"),  wxT("‚"),  true);
        strDecodedHtml.Replace(wxT("&fnof;"),   wxT("ƒ"),  true);
        strDecodedHtml.Replace(wxT("&bdquo;"),  wxT("„"),  true);
        strDecodedHtml.Replace(wxT("&hellip;"), wxT("…"),  true);
        strDecodedHtml.Replace(wxT("&dagger;"), wxT("†"),  true);
        strDecodedHtml.Replace(wxT("&Dagger;"), wxT("‡"),  true);
        strDecodedHtml.Replace(wxT("&Scaron;"), wxT("Š"),  true);
        strDecodedHtml.Replace(wxT("&OElig;"),  wxT("Œ"),  true);
        strDecodedHtml.Replace(wxT("&lsquo;"),  wxT("‘"),  true);
        strDecodedHtml.Replace(wxT("&rsquo;"),  wxT("’"),  true);
        strDecodedHtml.Replace(wxT("&ldquo;"),  wxT("“"),  true);
        strDecodedHtml.Replace(wxT("&rdquo;"),  wxT("”"),  true);
        strDecodedHtml.Replace(wxT("&bull;"),   wxT("•"),  true);
        strDecodedHtml.Replace(wxT("&ndash;"),  wxT("–"),  true);
        strDecodedHtml.Replace(wxT("&mdash;"),  wxT("—"),  true);
        strDecodedHtml.Replace(wxT("&tilde;"),  wxT("˜˜~"),  true);
        strDecodedHtml.Replace(wxT("&trade;"),  wxT("™"),  true);
        strDecodedHtml.Replace(wxT("&scaron;"), wxT("š"),  true);
        strDecodedHtml.Replace(wxT("&oelig;"),  wxT("œ"),  true);
        strDecodedHtml.Replace(wxT("&Yuml;"),   wxT("Ÿ"),  true);
        strDecodedHtml.Replace(wxT("&iexcl;"),  wxT("¡"),  true);
        strDecodedHtml.Replace(wxT("&cent;"),   wxT("¢"),  true);
        strDecodedHtml.Replace(wxT("&pound;"),  wxT("£"),  true);
        strDecodedHtml.Replace(wxT("&curren;"), wxT("¤"),  true);
        strDecodedHtml.Replace(wxT("&yen;"),    wxT("¥"),  true);
        strDecodedHtml.Replace(wxT("&brvbar;"), wxT("¦"),  true);
        strDecodedHtml.Replace(wxT("&sect;"),   wxT("§"),  true);
        strDecodedHtml.Replace(wxT("&uml;"),    wxT("¨"),  true);
        strDecodedHtml.Replace(wxT("&copy;"),   wxT("©"),  true);
        strDecodedHtml.Replace(wxT("&ordf;"),   wxT("ª"),  true);
        strDecodedHtml.Replace(wxT("&laquo;"),  wxT("«"),  true);
        strDecodedHtml.Replace(wxT("&not;"),    wxT("¬"),  true);
        strDecodedHtml.Replace(wxT("&reg;"),    wxT("®"),  true);
        strDecodedHtml.Replace(wxT("&macr;"),   wxT("¯"),  true);
        strDecodedHtml.Replace(wxT("&deg;"),    wxT("°"),  true);
        strDecodedHtml.Replace(wxT("&plusmn;"), wxT("±"),  true);
        strDecodedHtml.Replace(wxT("&sup2;"),   wxT("²"),  true);
        strDecodedHtml.Replace(wxT("&sup3;"),   wxT("³"),  true);
        strDecodedHtml.Replace(wxT("&acute;"),  wxT("´"),  true);
        strDecodedHtml.Replace(wxT("&micro;"),  wxT("µ"),  true);
        strDecodedHtml.Replace(wxT("&para;"),   wxT("¶"),  true);
        strDecodedHtml.Replace(wxT("&middot;"), wxT("·"),  true);
        strDecodedHtml.Replace(wxT("&cedil;"),  wxT("¸"),  true);
        strDecodedHtml.Replace(wxT("&sup1;"),   wxT("¹"),  true);
        strDecodedHtml.Replace(wxT("&ordm;"),   wxT("º"),  true);
        strDecodedHtml.Replace(wxT("&raquo;"),  wxT("»"),  true);
        strDecodedHtml.Replace(wxT("&frac14;"), wxT("¼"),  true);
        strDecodedHtml.Replace(wxT("&frac12;"), wxT("½"),  true);
        strDecodedHtml.Replace(wxT("&frac34;"), wxT("¾"),  true);
        strDecodedHtml.Replace(wxT("&iquest;"), wxT("¿"),  true);
        strDecodedHtml.Replace(wxT("&Agrave;"), wxT("À"),  true);
        strDecodedHtml.Replace(wxT("&Aacute;"), wxT("Á"),  true);
        strDecodedHtml.Replace(wxT("&Acirc;"),  wxT("Â"),  true);
        strDecodedHtml.Replace(wxT("&Atilde;"), wxT("Ã"),  true);
        strDecodedHtml.Replace(wxT("&Auml;"),   wxT("Ä"),  true);
        strDecodedHtml.Replace(wxT("&Aring;"),  wxT("Å"),  true);
        strDecodedHtml.Replace(wxT("&AElig;"),  wxT("Æ"),  true);
        strDecodedHtml.Replace(wxT("&Ccedil;"), wxT("Ç"),  true);
        strDecodedHtml.Replace(wxT("&Egrave;"), wxT("È"),  true);
        strDecodedHtml.Replace(wxT("&Eacute;"), wxT("É"),  true);
        strDecodedHtml.Replace(wxT("&Ecirc;"),  wxT("Ê"),  true);
        strDecodedHtml.Replace(wxT("&Euml;"),   wxT("Ë"),  true);
        strDecodedHtml.Replace(wxT("&Igrave;"), wxT("Ì"),  true);
        strDecodedHtml.Replace(wxT("&Iacute;"), wxT("Í"),  true);
        strDecodedHtml.Replace(wxT("&Icirc;"),  wxT("Î"),  true);
        strDecodedHtml.Replace(wxT("&Iuml;"),   wxT("Ï"),  true);
        strDecodedHtml.Replace(wxT("&ETH;"),    wxT("Ð"),  true);
        strDecodedHtml.Replace(wxT("&Ntilde;"), wxT("Ñ"),  true);
        strDecodedHtml.Replace(wxT("&Ograve;"), wxT("Ò"),  true);
        strDecodedHtml.Replace(wxT("&Oacute;"), wxT("Ó"),  true);
        strDecodedHtml.Replace(wxT("&Ocirc;"),  wxT("Ô"),  true);
        strDecodedHtml.Replace(wxT("&Otilde;"), wxT("Õ"),  true);
        strDecodedHtml.Replace(wxT("&Ouml;"),   wxT("Ö"),  true);
        strDecodedHtml.Replace(wxT("&times;"),  wxT("×"),  true);
        strDecodedHtml.Replace(wxT("&Oslash;"), wxT("Ø"),  true);
        strDecodedHtml.Replace(wxT("&Ugrave;"), wxT("Ù"),  true);
        strDecodedHtml.Replace(wxT("&Uacute;"), wxT("Ú"),  true);
        strDecodedHtml.Replace(wxT("&Ucirc;"),  wxT("Û"),  true);
        strDecodedHtml.Replace(wxT("&Uuml;"),   wxT("Ü"),  true);
        strDecodedHtml.Replace(wxT("&Yacute;"), wxT("Ý"),  true);
        strDecodedHtml.Replace(wxT("&THORN;"),  wxT("Þ"),  true);
        strDecodedHtml.Replace(wxT("&szlig;"),  wxT("ß"),  true);
        strDecodedHtml.Replace(wxT("&agrave;"), wxT("à"),  true);
        strDecodedHtml.Replace(wxT("&aacute;"), wxT("á"),  true);
        strDecodedHtml.Replace(wxT("&acirc;"),  wxT("â"),  true);
        strDecodedHtml.Replace(wxT("&atilde;"), wxT("ã"),  true);
        strDecodedHtml.Replace(wxT("&auml;"),   wxT("ä"),  true);
        strDecodedHtml.Replace(wxT("&aring;"),  wxT("å"),  true);
        strDecodedHtml.Replace(wxT("&aelig;"),  wxT("æ"),  true);
        strDecodedHtml.Replace(wxT("&ccedil;"), wxT("ç"),  true);
        strDecodedHtml.Replace(wxT("&egrave;"), wxT("è"),  true);
        strDecodedHtml.Replace(wxT("&eacute;"), wxT("é"),  true);
        strDecodedHtml.Replace(wxT("&ecirc;"),  wxT("ê"),  true);
        strDecodedHtml.Replace(wxT("&euml;"),   wxT("ë"),  true);
        strDecodedHtml.Replace(wxT("&igrave;"), wxT("ì"),  true);
        strDecodedHtml.Replace(wxT("&iacute;"), wxT("í"),  true);
        strDecodedHtml.Replace(wxT("&icirc;"),  wxT("î"),  true);
        strDecodedHtml.Replace(wxT("&iuml;"),   wxT("ï"),  true);
        strDecodedHtml.Replace(wxT("&eth;"),    wxT("ð"),  true);
        strDecodedHtml.Replace(wxT("&ntilde;"), wxT("ñ"),  true);
        strDecodedHtml.Replace(wxT("&ograve;"), wxT("ò"),  true);
        strDecodedHtml.Replace(wxT("&oacute;"), wxT("ó"),  true);
        strDecodedHtml.Replace(wxT("&ocirc;"),  wxT("ô"),  true);
        strDecodedHtml.Replace(wxT("&otilde;"), wxT("õ"),  true);
        strDecodedHtml.Replace(wxT("&ouml;"),   wxT("ö"),  true);
        strDecodedHtml.Replace(wxT("&divide;"), wxT("÷"),  true);
        strDecodedHtml.Replace(wxT("&oslash;"), wxT("ø"),  true);
        strDecodedHtml.Replace(wxT("&ugrave;"), wxT("ù"),  true);
        strDecodedHtml.Replace(wxT("&uacute;"), wxT("ú"),  true);
        strDecodedHtml.Replace(wxT("&ucirc;"),  wxT("û"),  true);
        strDecodedHtml.Replace(wxT("&uuml;"),   wxT("ü"),  true);
        strDecodedHtml.Replace(wxT("&yacute;"), wxT("ý"),  true);
        strDecodedHtml.Replace(wxT("&thorn;"),  wxT("þ"),  true);
        strDecodedHtml.Replace(wxT("&yuml;"),   wxT("ÿ"),  true);
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
