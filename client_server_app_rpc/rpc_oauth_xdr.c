/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include "rpc_oauth.h"

bool_t
xdr_error (XDR *xdrs, error *objp)
{
	register int32_t *buf;

	 if (!xdr_enum (xdrs, (enum_t *) objp))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_operation (XDR *xdrs, operation *objp)
{
	register int32_t *buf;

	 if (!xdr_enum (xdrs, (enum_t *) objp))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_req_auth (XDR *xdrs, req_auth *objp)
{
	register int32_t *buf;

	 if (!xdr_string (xdrs, &objp->user_id, ~0))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_res_auth (XDR *xdrs, res_auth *objp)
{
	register int32_t *buf;

	 if (!xdr_string (xdrs, &objp->auth_token, ~0))
		 return FALSE;
	 if (!xdr_error (xdrs, &objp->err))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_req_acc_token (XDR *xdrs, req_acc_token *objp)
{
	register int32_t *buf;

	 if (!xdr_string (xdrs, &objp->user_id, ~0))
		 return FALSE;
	 if (!xdr_string (xdrs, &objp->auth_token, ~0))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_res_acc_token (XDR *xdrs, res_acc_token *objp)
{
	register int32_t *buf;

	 if (!xdr_string (xdrs, &objp->acc_token, ~0))
		 return FALSE;
	 if (!xdr_string (xdrs, &objp->regen_acc_token, ~0))
		 return FALSE;
	 if (!xdr_int (xdrs, &objp->ttl))
		 return FALSE;
	 if (!xdr_error (xdrs, &objp->err))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_req_val_del_act (XDR *xdrs, req_val_del_act *objp)
{
	register int32_t *buf;

	 if (!xdr_operation (xdrs, &objp->op))
		 return FALSE;
	 if (!xdr_string (xdrs, &objp->resource, ~0))
		 return FALSE;
	 if (!xdr_string (xdrs, &objp->acc_token, ~0))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_res_val_del_act (XDR *xdrs, res_val_del_act *objp)
{
	register int32_t *buf;

	 if (!xdr_error (xdrs, &objp->err))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_req_approve_req_token (XDR *xdrs, req_approve_req_token *objp)
{
	register int32_t *buf;

	 if (!xdr_string (xdrs, &objp->auth_token, ~0))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_res_approve_req_token (XDR *xdrs, res_approve_req_token *objp)
{
	register int32_t *buf;

	 if (!xdr_string (xdrs, &objp->auth_token, ~0))
		 return FALSE;
	return TRUE;
}
