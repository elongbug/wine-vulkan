/*
 * Copyright 2005 Jacek Caban for CodeWeavers
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "config.h"

#include <stdarg.h>
#include <stdio.h>

#define COBJMACROS

#include "windef.h"
#include "winbase.h"
#include "winuser.h"
#include "ole2.h"

#include "wine/debug.h"

#include "mshtml_private.h"

WINE_DEFAULT_DEBUG_CHANNEL(mshtml);

#define HTMLDOC3_THIS(iface) DEFINE_THIS(HTMLDocument, HTMLDocument3, iface)

static HRESULT WINAPI HTMLDocument3_QueryInterface(IHTMLDocument3 *iface,
                                                  REFIID riid, void **ppv)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    return IHTMLDocument2_QueryInterface(HTMLDOC(This), riid, ppv);
}

static ULONG WINAPI HTMLDocument3_AddRef(IHTMLDocument3 *iface)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    return IHTMLDocument2_AddRef(HTMLDOC(This));
}

static ULONG WINAPI HTMLDocument3_Release(IHTMLDocument3 *iface)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    return IHTMLDocument2_Release(HTMLDOC(This));
}

static HRESULT WINAPI HTMLDocument3_GetTypeInfoCount(IHTMLDocument3 *iface, UINT *pctinfo)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->(%p)\n", This, pctinfo);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_GetTypeInfo(IHTMLDocument3 *iface, UINT iTInfo,
                                                LCID lcid, ITypeInfo **ppTInfo)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->(%u %lu %p)\n", This, iTInfo, lcid, ppTInfo);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_GetIDsOfNames(IHTMLDocument3 *iface, REFIID riid,
                                                LPOLESTR *rgszNames, UINT cNames,
                                                LCID lcid, DISPID *rgDispId)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->(%s %p %u %lu %p)\n", This, debugstr_guid(riid), rgszNames, cNames,
                                        lcid, rgDispId);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_Invoke(IHTMLDocument3 *iface, DISPID dispIdMember,
                            REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams,
                            VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->(%ld %s %ld %d %p %p %p %p)\n", This, dispIdMember, debugstr_guid(riid),
            lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_releaseCapture(IHTMLDocument3 *iface)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)\n", This);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_recalc(IHTMLDocument3 *iface, VARIANT_BOOL fForce)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->(%x)\n", This, fForce);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_createTextNode(IHTMLDocument3 *iface, BSTR text,
                                                   IHTMLDOMNode **newTextNode)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->(%s %p)\n", This, debugstr_w(text), newTextNode);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_get_documentElement(IHTMLDocument3 *iface, IHTMLElement **p)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    nsIDOMDocument *nsdoc;
    HTMLDOMNode *node;
    nsresult nsres;

    TRACE("(%p)->(%p)\n", This, p);

    if(!This->nscontainer) {
        *p = NULL;
        return S_OK;
    }

    nsres = nsIWebNavigation_GetDocument(This->nscontainer->navigation, &nsdoc);
    if(NS_FAILED(nsres))
        ERR("GetDocument failed: %08lx\n", nsres);

    if(nsdoc) {
        node = get_node(This, (nsIDOMNode*)nsdoc);
        nsIDOMDocument_Release(nsdoc);

        IHTMLDOMNode_QueryInterface(HTMLDOMNODE(node), &IID_IHTMLElement, (void**)p);
    }else {
        *p = NULL;
    }

    return S_OK;
}

static HRESULT WINAPI HTMLDocument3_uniqueID(IHTMLDocument3 *iface, BSTR *p)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->(%p)\n", This, p);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_attachEvent(IHTMLDocument3 *iface, BSTR event,
                                                IDispatch* pDisp, VARIANT_BOOL *pfResult)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->(%s %p %p)\n", This, debugstr_w(event), pDisp, pfResult);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_detachEvent(IHTMLDocument3 *iface, BSTR event,
                                                IDispatch *pDisp)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->(%s %p)\n", This, debugstr_w(event), pDisp);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_put_onrowsdelete(IHTMLDocument3 *iface, VARIANT v)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->()\n", This);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_get_onrowsdelete(IHTMLDocument3 *iface, VARIANT *p)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->(%p)\n", This, p);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_put_onrowsinserted(IHTMLDocument3 *iface, VARIANT v)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->()\n", This);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_get_onrowsinserted(IHTMLDocument3 *iface, VARIANT *p)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->(%p)\n", This, p);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_put_oncellchange(IHTMLDocument3 *iface, VARIANT v)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->()\n", This);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_get_oncellchange(IHTMLDocument3 *iface, VARIANT *p)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->(%p)\n", This, p);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_put_ondatasetchanged(IHTMLDocument3 *iface, VARIANT v)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->()\n", This);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_get_ondatasetchanged(IHTMLDocument3 *iface, VARIANT *p)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->(%p)\n", This, p);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_put_ondataavailable(IHTMLDocument3 *iface, VARIANT v)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->()\n", This);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_get_ondataavailable(IHTMLDocument3 *iface, VARIANT *p)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->(%p)\n", This, p);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_put_ondatasetcomplete(IHTMLDocument3 *iface, VARIANT v)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->()\n", This);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_get_ondatasetcomplete(IHTMLDocument3 *iface, VARIANT *p)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->(%p)\n", This, p);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_put_onpropertychange(IHTMLDocument3 *iface, VARIANT v)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->()\n", This);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_get_onpropertychange(IHTMLDocument3 *iface, VARIANT *p)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->(%p)\n", This, p);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_put_dir(IHTMLDocument3 *iface, BSTR v)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->(%s)\n", This, debugstr_w(v));
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_get_dir(IHTMLDocument3 *iface, BSTR *p)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->(%p)\n", This, p);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_put_oncontextmenu(IHTMLDocument3 *iface, VARIANT v)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->()\n", This);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_get_oncontextmenu(IHTMLDocument3 *iface, VARIANT *p)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->(%p)\n", This, p);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_put_onstop(IHTMLDocument3 *iface, VARIANT v)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->()\n", This);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_get_onstop(IHTMLDocument3 *iface, VARIANT *p)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->(%p)\n", This, p);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_createDocumentFragment(IHTMLDocument3 *iface,
                                                           IHTMLDocument2 **ppNewDoc)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->(%p)\n", This, ppNewDoc);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_get_parentDocument(IHTMLDocument3 *iface,
                                                       IHTMLDocument2 **p)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->(%p)\n", This, p);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_put_enableDownload(IHTMLDocument3 *iface,
                                                       VARIANT_BOOL v)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->(%x)\n", This, v);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_get_enableDownload(IHTMLDocument3 *iface,
                                                       VARIANT_BOOL *p)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->(%p)\n", This, p);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_put_baseUrl(IHTMLDocument3 *iface, BSTR v)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->(%s)\n", This, debugstr_w(v));
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_get_baseUrl(IHTMLDocument3 *iface, BSTR *p)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->(%p)\n", This, p);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_get_childNodes(IHTMLDocument3 *iface, IDispatch **p)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->(%p)\n", This, p);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_put_inheritStyleSheets(IHTMLDocument3 *iface,
                                                           VARIANT_BOOL v)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->()\n", This);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_get_inheritStyleSheets(IHTMLDocument3 *iface,
                                                           VARIANT_BOOL *p)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->(%p)\n", This, p);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_put_onbeforeeditfocus(IHTMLDocument3 *iface, VARIANT v)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->()\n", This);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_get_onbeforeeditfocus(IHTMLDocument3 *iface, VARIANT *p)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->(%p)\n", This, p);
    return E_NOTIMPL;
}

static HRESULT WINAPI HTMLDocument3_getElementsByName(IHTMLDocument3 *iface, BSTR v,
                                                      IHTMLElementCollection **ppelColl)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->(%s %p)\n", This, debugstr_w(v), ppelColl);
    return E_NOTIMPL;
}


static HRESULT WINAPI HTMLDocument3_getElementById(IHTMLDocument3 *iface, BSTR v,
                                                   IHTMLElement **pel)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->(%s %p)\n", This, debugstr_w(v), pel);
    return E_NOTIMPL;
}


static HRESULT WINAPI HTMLDocument3_getElementsByTagName(IHTMLDocument3 *iface, BSTR v,
                                                         IHTMLElementCollection **pelColl)
{
    HTMLDocument *This = HTMLDOC3_THIS(iface);
    FIXME("(%p)->(%s %p)\n", This, debugstr_w(v), pelColl);
    return E_NOTIMPL;
}

static const IHTMLDocument3Vtbl HTMLDocument3Vtbl = {
    HTMLDocument3_QueryInterface,
    HTMLDocument3_AddRef,
    HTMLDocument3_Release,
    HTMLDocument3_GetTypeInfoCount,
    HTMLDocument3_GetTypeInfo,
    HTMLDocument3_GetIDsOfNames,
    HTMLDocument3_Invoke,
    HTMLDocument3_releaseCapture,
    HTMLDocument3_recalc,
    HTMLDocument3_createTextNode,
    HTMLDocument3_get_documentElement,
    HTMLDocument3_uniqueID,
    HTMLDocument3_attachEvent,
    HTMLDocument3_detachEvent,
    HTMLDocument3_put_onrowsdelete,
    HTMLDocument3_get_onrowsdelete,
    HTMLDocument3_put_onrowsinserted,
    HTMLDocument3_get_onrowsinserted,
    HTMLDocument3_put_oncellchange,
    HTMLDocument3_get_oncellchange,
    HTMLDocument3_put_ondatasetchanged,
    HTMLDocument3_get_ondatasetchanged,
    HTMLDocument3_put_ondataavailable,
    HTMLDocument3_get_ondataavailable,
    HTMLDocument3_put_ondatasetcomplete,
    HTMLDocument3_get_ondatasetcomplete,
    HTMLDocument3_put_onpropertychange,
    HTMLDocument3_get_onpropertychange,
    HTMLDocument3_put_dir,
    HTMLDocument3_get_dir,
    HTMLDocument3_put_oncontextmenu,
    HTMLDocument3_get_oncontextmenu,
    HTMLDocument3_put_onstop,
    HTMLDocument3_get_onstop,
    HTMLDocument3_createDocumentFragment,
    HTMLDocument3_get_parentDocument,
    HTMLDocument3_put_enableDownload,
    HTMLDocument3_get_enableDownload,
    HTMLDocument3_put_baseUrl,
    HTMLDocument3_get_baseUrl,
    HTMLDocument3_get_childNodes,
    HTMLDocument3_put_inheritStyleSheets,
    HTMLDocument3_get_inheritStyleSheets,
    HTMLDocument3_put_onbeforeeditfocus,
    HTMLDocument3_get_onbeforeeditfocus,
    HTMLDocument3_getElementsByName,
    HTMLDocument3_getElementById,
    HTMLDocument3_getElementsByTagName
};

void HTMLDocument_HTMLDocument3_Init(HTMLDocument *This)
{
    This->lpHTMLDocument3Vtbl = &HTMLDocument3Vtbl;
}
