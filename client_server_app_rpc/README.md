In this README, I will present and explain the role of each structure in both the client and the server. Regarding the written routes and functions, the four required routes are presented, each performing exactly what was requested in the assignment.

To begin with, on the server, I first parse each input file. I do this at the beginning to open the input files only once, avoiding the need to read each file for every request.
  ~ In the vector<string> users_in_database, I save the users from the database to check in request_authorization_1_svc if a user is in the database.
  ~ In the vector<string> resources, I save the resources from the database to check in validate_delegated_action_1_svc if a resource is in the database.
  ~ The queue<map<string, vector<int>>> resource_operations_queue is a queue populated after parsing the approvals.db file. Each element in the queue is a map linking a resource to its associated permissions. I use this queue in validate_delegated_action_1_svc to see if the actions requested by the client are permitted.
  ~ The map<string, Client*> user_id_clients is a map I use throughout the assignment, linking a user_id to an instance of the Client class. In the Client class, I have all the current client information. The idea of creating this class came when I realized that generating the access token is based on the authentication token, not the signed token received from the client. This issue requires keeping track of multiple client details for each client, making it more useful to store all client information in a class.

In the client, I use only the structure map<string, string> user_id_access_token because after each REQUEST command, I want to associate the user_id with the access token, which is necessary for future validate delegate action requests where the client needs to send the access token received from the REQUEST commands to the server.

Note: I removed the main function from rpc_oauth_svc.c and added its content to the server's main because, during server compilation, I was receiving an error indicating this.
  
Nicolae Paul-Gabriel
343C3
