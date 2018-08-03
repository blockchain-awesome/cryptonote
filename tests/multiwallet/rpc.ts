import * as request from "request-promise";

async function requestRPC(url, json) {
  const options = {
    uri: url,
    method: 'POST',
    json: json
  };
  return request(options);
}


requestRPC("http://127.0.0.1:19888/json_rpc", {
  method: 'login',
  params: {
    address: 'BH5DXaf3TxWZpPDdTuDct4XfrvXp652Hj6KupGU7UF2LfigBsrQeP9xCW21Ni2es5Y5hVPjn7D2XzKewYHvBWJXY1rw9axs',
    sendSecretKey: '57226f217f74a817e6899a10c31d1bbd72a36bf36fab79e39065afc2c9aa9900',
    viewSecretKey: "1b62fd3477af6fd7c949c326d372787a9dc645b3aa2e735168f91e55e807bd0d"
  }
}).then((body) => {
  console.log(body);
});

requestRPC("http://127.0.0.1:19888/json_rpc", {
  method: 'login',
  params: {
    address: 'BH5DXaf3TxWZpPDdTuDct4XfrvXp652Hj6KupGU7UF2LfigBsrQeP9xCW21Ni2es5Y5hVPjn7D2XzKewYHvBWJXY1rw9axs',
    sendSecretKey: '57226f217f74a817e6899a10c31d1bbd72a36bf36fab79e39065afc2c9aa9900',
    viewSecretKey: "1b62fd3487af6fd7c949c326d372787a9dc645b3aa2e735168f91e55e807bd0d"
  }
}).then((body) => {
  console.log(body);
});

