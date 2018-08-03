import * as request from "request-promise";

async function requestRPC(url, json) {
  const options = {
    uri: url,
    method: 'POST',
    json: json
  };
  return request(options);
}
function run() {

  return async function () {

    const login = await requestRPC("http://127.0.0.1:19888/json_rpc", {
      method: 'login',
      params: {
        address: 'BH5DXaf3TxWZpPDdTuDct4XfrvXp652Hj6KupGU7UF2LfigBsrQeP9xCW21Ni2es5Y5hVPjn7D2XzKewYHvBWJXY1rw9axs',
        sendSecretKey: '57226f217f74a817e6899a10c31d1bbd72a36bf36fab79e39065afc2c9aa9900',
        viewSecretKey: "1b62fd3477af6fd7c949c326d372787a9dc645b3aa2e735168f91e55e807bd0d"
      }
    });

    const token = login.result.token;

    console.log(login);

    const wrongLogin = await requestRPC("http://127.0.0.1:19888/json_rpc", {
      method: 'login',
      params: {
        address: 'BH5DXaf3TxWZpPDdTuDct4XfrvXp652Hj6KupGU7UF2LfigBsrQeP9xCW21Ni2es5Y5hVPjn7D2XzKewYHvBWJXY1rw9axs',
        sendSecretKey: '57226f217f74a817e6899a10c31d1bbd72a36bf36fab79e39065afc2c9aa9900',
        viewSecretKey: "1b62fd3487af6fd7c949c326d372787a9dc645b3aa2e735168f91e55e807bd0d"
      }
    });

    console.log(wrongLogin);

    const balance = await requestRPC("http://127.0.0.1:19888/json_rpc", {
      method: 'getBalance',
      params: {
        token: token,
      }
    });
    console.log(balance);
  }
}


run()().then(() => {
  console.log("request end");
});