//
// Copyright (c) 2008 Tridium, Inc.
// Licensed under the Academic Free License version 3.0
//
// History:
//   10 Mar 08  Brian Frank  Creation
//   09 Jul 10  Matthew Giannini 
//      rename to DaspSessionMessage and extend DaspMessage
//

package sedona.dasp;

import java.io.UnsupportedEncodingException;

/**
 * DaspSessionMessage models a DASP message and is bound to a DaspSession.
 */
public class DaspSessionMessage extends DaspMessage
{
  DaspSessionMessage() 
  {
    super();
  }
  
  DaspSessionMessage(byte[] buf, int length)
    throws UnsupportedEncodingException
  {
    super(buf, length);
  }

  /**
   * Get the session this message was received from.
   */
  public DaspSession session() { return session; }

  /**
   * String representation.
   */
  public String toString()
  {
    return "DaspSessionMessage @ " + session + " (" + payload.length + " bytes)";
  }

////////////////////////////////////////////////////////////////
// Fields
////////////////////////////////////////////////////////////////

  // next in Queue linked list
  DaspSessionMessage next;

  // DaspSession
  DaspSession session;
}
