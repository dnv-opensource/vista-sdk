import React, {Component} from 'react';
import logo from './logo.svg';
import './App.css';


// const gmod = VIS.instance.getGmod(VisVersion.v3_5a);
function App() {
    const url =  "https://mavista.azureedge.net/vis/gmod-vis-3-4a.json";
    const options: RequestInit = {
        method: "GET",
        mode: "no-cors",
        headers: {
            "Content-Type": "application/json",
            // "Cache-Control": "public, max-age=3600, must-revalidate"
    }, cache: 'force-cache'
    };
    setTimeout(() => {
    const response = fetch(url, options);
    response.then(r => {
        console.log(r.headers);
    });
    }, 200);
  return (
<div>

</div>


  );
}
export default App;
