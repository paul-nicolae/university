// error enum, used to return errors from the server to the client
enum error {
    USER_NOT_FOUND,
    REQUEST_DENIED,
    PERMISSION_DENIED,
    TOKEN_EXPIRED,
    RESOURCE_NOT_FOUND,
    OPERATION_NOT_PERMITTED,
    PERMISSION_GRANTED,
    NO_ERROR
};

// operation enum, used to specify the operation that the client wants to perform
enum operation {
    READ,
    INSERT,
    MODIFY,
    DELETE,
    EXECUTE
};

// request authorization struct, used to request authorization from the server
struct req_auth {
    string user_id<>;
};

// response authorization struct, used to return the authorization token
// to the client, or USER_NOT_FOUND error
struct res_auth {
    string auth_token<>;
    error err;
};

// request access token struct, used to request an access token from the server
struct req_acc_token {
    string user_id<>;
    string auth_token<>;
};

// response access token struct, used to return the access token
// to the client, regen access token, ttl, or REQUEST_DENIED error
struct res_acc_token {
    string acc_token<>;
    string regen_acc_token<>;
    int ttl;
    error err;
};

// request validate delegated action struct, used to request validation
// of a delegated action from the server
struct req_val_del_act {
    operation op;
    string resource<>;
    string acc_token<>;
};

// response validate delegated action struct, used to return the result
// of the validation to the client, or an error in case of failure
struct res_val_del_act {
    error err;
};

// request approve request token struct, used to request approval
// of a request token from the server
struct req_approve_req_token {
    string auth_token<>;
};

// response approve request token struct, used to return the result
// of the approval to the client, or an error in case of failure
struct res_approve_req_token {
    string auth_token<>;
};

program RPC_OAUTH_PROG {
    version RPC_OAUTH_VERS { 
        
        res_auth request_authorization(req_auth) = 1; // request authorization from the server

        res_acc_token request_access_token(req_acc_token) = 2; // request access token from the server

        res_val_del_act validate_delegated_action(req_val_del_act) = 3; // validate delegated action

        res_approve_req_token approve_request_token(req_approve_req_token) = 4; // approve request token
    } = 1;
} = 1;