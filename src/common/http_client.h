#pragma once

#include <wx/curl/http.h>

namespace marine_navi {

namespace Utils {

class HttpClient : public wxCurlHTTP {
public:
  HttpClient(const wxString& szURL = wxEmptyString,
             const wxString& szUserName = wxEmptyString,
             const wxString& szPassword = wxEmptyString,
             wxEvtHandler* pEvtHandler = NULL, int id = wxID_ANY,
             long flags = wxCURL_DEFAULT_FLAGS);
  ~HttpClient() override = default;

  void AddHeader(const std::string& value);
};

}  // namespace Utils
}  // namespace marine_navi