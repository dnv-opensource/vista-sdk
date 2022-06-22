import React, {Component} from 'react';
import logo from './logo.svg';
import './App.css';
import gmodjson from 'dnv-vista-sdk/dist/resources/gmod-vis-3-4a.json'
import {VIS, VisVersion} from 'dnv-vista-sdk'


const gmod = VIS.instance.getGmod(VisVersion.v3_5a);
function App() {
  return (
<div>

{JSON.stringify(Promise.resolve(gmod))}
</div>
  );
}
export default App;

// class LocalFileRead extends Component {
//     constructor(props) {
//         super(props);
//     }
//     render() {
//         return <div>

//             {JSON.stringify(gmod)}
//             </div>
//     }
// }
// export default LocalFileRead;
