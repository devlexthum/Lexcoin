%% -*- erlang -*-
{application, blockchain_node,
 [
   {description, "A Lexthum Blockchain Node"},
   {vsn, "1.0.0"},
   {registered, []},
   {mod, {blockchain_node_app, []}},
   {
    applications,
    [
     kernel,
     stdlib,
     syntax_tools,
     compiler,
     lager,
     jsone,
     jsonrpc2,
     base64url,
     pbkdf2,
     observer_cli,
     clique,
     recon,
     grpcbox,
     elli,
     telemetry,
     prometheus
    ]
   },
   {included_applications, [blockchain]},
   {env, []},
   {modules, []},
   {licenses, ["Apache 2.0"]},
   {links, []}
 ]
}.