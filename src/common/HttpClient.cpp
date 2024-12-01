#include "HttpClient.h"

namespace MarineNavi {

namespace Utils {

HttpClient::HttpClient(const wxString& szURL, const wxString& szUserName,
                       const wxString& szPassword, wxEvtHandler* pEvtHandler,
                       int id, long flags)
    : wxCurlHTTP(szURL, szUserName, szPassword, pEvtHandler, id, flags) {}

void HttpClient::AddHeader(const std::string& value) {
  m_arrHeaders.Add(value);
}

}  // namespace Utils
}  // namespace MarineNavi