#include "PlexConnection.h"

#include "filesystem/CurlFile.h"

using namespace XFILE;

CPlexConnection::CPlexConnection(int type, const CStdString& host, int port, const CStdString& token) :
  m_type(type), m_state(CONNECTION_STATE_UNKNOWN), m_token(token)
{
  m_url.SetHostName(host);
  m_url.SetPort(port);
  if (port == 443)
    m_url.SetProtocol("https");
  else
    m_url.SetProtocol("http");
}

CURL
CPlexConnection::BuildURL(const CStdString &path)
{
  CURL ret(m_url);
  ret.SetFileName(path);
  return ret;
}

CPlexConnection::ConnectionState
CPlexConnection::TestReachability(CPlexServerPtr server)
{
  CCurlFile http;
  CURL url = BuildURL("/");
  CStdString rootXml;

  if (http.Get(url.Get(), rootXml))
  {
    server->CollectDataFromRoot(rootXml);
    m_state = CONNECTION_STATE_REACHABLE;
  }
  else
  {
    if (http.GetLastHTTPResponseCode() == 401)
      m_state = CONNECTION_STATE_UNAUTHORIZED;
    else
      m_state = CONNECTION_STATE_UNREACHABLE;
  }

  return m_state;
}

void
CPlexConnection::Merge(CPlexConnectionPtr otherConnection)
{
  m_url = otherConnection->m_url;
  m_token = otherConnection->m_token;
  m_type |= otherConnection->m_type;
  m_refreshed = true;
}

CStdString
CPlexConnection::ConnectionStateName(CPlexConnection::ConnectionState state)
{
  switch (state) {
    case CONNECTION_STATE_REACHABLE:
      return "reachable";
      break;
    case CONNECTION_STATE_UNAUTHORIZED:
      return "unauthorized";
      break;
    case CONNECTION_STATE_UNKNOWN:
      return "unknown";
      break;
    default:
      return "unreachable";
      break;
  }
}

CStdString
CPlexConnection::ConnectionTypeName(CPlexConnection::ConnectionType type)
{
  CStdString typeName;
  if (type & CONNECTION_DISCOVERED)
    typeName = "(discovered)";
  if (type & CONNECTION_MANUAL)
    typeName += "(manual)";
  if (type & CONNECTION_MYPLEX)
    typeName += "(myplex)";

  return typeName;
}