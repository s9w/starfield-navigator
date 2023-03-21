import * as THREE from 'three';
import { CSS2DRenderer, CSS2DObject } from 'three/addons/renderers/CSS2DRenderer.js';
import { OrbitControls } from 'three/addons/controls/OrbitControls.js';

let container = document.getElementById('glContainer');
let camera = new THREE.PerspectiveCamera( 90, container.clientWidth / container.clientHeight, 0.1, 1000 );
let renderer = new THREE.WebGLRenderer({antialias: true});
let labelRenderer = new CSS2DRenderer();

const up_vec = new THREE.Vector3(-0.484225601, 0.746894360, 0.455712944);
const right_vec = new THREE.Vector3(0.0548099577, -0.493931174, 0.867771626);
const front_vec = new THREE.Vector3(0.873337090, 0.445001483, 0.198131084);

function get_line_segment(center, angle, radius)
{
    let point = center.clone();
    let front_shift = front_vec.clone();
    front_shift.multiplyScalar(radius * Math.cos(angle));
    point.add(front_shift);
    let right_shift = right_vec.clone();
    right_shift.multiplyScalar(radius * Math.sin(angle));
    point.add(right_shift);
    return point;
}

function add_ring(line_points, center, radius)
{
    const n = 32;
    for (let i = 0; i < n; i++) {
        const angle0 = 1.0*i/n*2.0*Math.PI;
        const angle1 = 1.0*(i+1)/n*2.0*Math.PI;
        line_points.push(get_line_segment(center, angle0, radius));
        line_points.push(get_line_segment(center, angle1, radius));
    }
}

function init() {
    const scene = new THREE.Scene();
    
    camera.up = up_vec;
    camera.position.add(front_vec);
    camera.position.multiplyScalar(-20.0);
    camera.lookAt(0, 0, 0);

    const material = new THREE.MeshBasicMaterial( { color: 0xffffff } );
    // scene.add( new THREE.Mesh( new THREE.SphereGeometry( 0.2, 32, 16 ).translate(1, 0, 0), new THREE.MeshBasicMaterial( { color: 0xff0000 } ) ) );
    // scene.add( new THREE.Mesh( new THREE.SphereGeometry( 0.2, 32, 16 ).translate(0, 1, 0), new THREE.MeshBasicMaterial( { color: 0x00ff00 } ) ) );
    // scene.add( new THREE.Mesh( new THREE.SphereGeometry( 0.2, 32, 16 ).translate(0, 0, 1), new THREE.MeshBasicMaterial( { color: 0x0000ff } ) ) );

    const star_radius = 0.5;

    const json_str = '[{"pos": [-3.9143505,6.3832874,-11.728529], "name": "van Maanens Star"}, {"pos": [39.047153,47.84092,11.375448], "name": "110 Herculis"}, {"pos": [60.7304,56.90959,15.325161], "name": "HD 171067"}, {"pos": [-3.6138592,-10.378395,-16.364534], "name": "e Eridani"}, {"pos": [11.158342,12.276173,-2.58902], "name": "Altair"}, {"pos": [41.147038,9.24025,-5.036249], "name": "HD 172051"}, {"pos": [1.5147209,11.22348,-1.1606045], "name": "61 Cygni"}, {"pos": [53.571194,27.16505,5.635515], "name": "Eta Serpentis"}, {"pos": [38.495026,23.419071,5.4212646], "name": "GJ 4056"}, {"pos": [14.575405,-8.465086,-10.686926], "name": "Delta Pavonis"}, {"pos": [8.850784,21.612757,8.20643], "name": "Vega"}, {"pos": [-3.4527874,-9.742649,-7.5088882], "name": "Kapteyns Star"}, {"pos": [-25.262615,48.472298,-3.1304095], "name": "Beta Cassiopeiae"}, {"pos": [14.040045,8.063974,3.2523985], "name": "70 Ophiuchi"}, {"pos": [-16.903536,28.029345,2.5367982], "name": "V547 Cassiopeiae"}, {"pos": [69.53643,30.59032,12.828529], "name": "Zeta Serpentis"}, {"pos": [0.0059432983,0.0034389496,0.0010299683], "name": "Sol"}, {"pos": [-14.128148,19.903185,-3.3746421], "name": "Mu Cassiopeiae"}, {"pos": [4.9491596,2.9781427,1.4466939], "name": "Barnard"}, {"pos": [-11.973907,-4.5371766,-10.021335], "name": "XXX"}, {"pos": [-10.427576,16.29337,-1.7111825], "name": "Eta Cassiopeiae"}, {"pos": [-3.3700757,0.4077597,-11.415973], "name": "Tau Ceti"}, {"pos": [14.072884,-4.294489,-1.7496204], "name": "Gliese 674"}, {"pos": [14.944046,19.43419,11.742174], "name": "Mu Herculis"}, {"pos": [-5.7810764,-6.241003,-1.3303785], "name": "Sirius"}, {"pos": [3.0883923,-3.0220547,-0.057224274], "name": "Alpha Centauri"}, {"pos": [22.490248,-3.242485,0.5599899], "name": "Gliese 667"}, {"pos": [19.357376,-0.5651779,2.3322744], "name": "36 Ophiuchi"}, {"pos": [57.105362,5.943104,4.2410755], "name": "58 Ophiuchi"}, {"pos": [-10.313726,-6.5316544,2.227744], "name": "Luytens Star"}, {"pos": [-9.2705555,-6.176506,2.5734143], "name": "Procyon"}, {"pos": [36.383083,26.33033,18.65766], "name": "Alpha Ophiuchi"}, {"pos": [68.16293,38.104767,24.164846], "name": "Beta Ophiuchi"}, {"pos": [47.20398,19.454292,16.034801], "name": "HD 158614"}, {"pos": [29.206533,10.18672,11.931849], "name": "Gliese 660"}, {"pos": [-1.9077282,-3.8985043,6.4825234], "name": "Wolf 359"}, {"pos": [55.830147,3.481554,8.788578], "name": "Xi Ophiuchi"}, {"pos": [16.555256,21.563265,22.845188], "name": "Zeta Herculis"}, {"pos": [14.952068,-6.013838,10.351568], "name": "HD 131977"}, {"pos": [9.632282,4.1092243,19.19151], "name": "Xi Bootis"}, {"pos": [57.78287,21.922012,27.23305], "name": "V2213 Ophiuchi"}, {"pos": [47.948254,15.921007,24.741491], "name": "HD 152391"}, {"pos": [39.39998,3.2160454,22.066975], "name": "18 Scorpii"}, {"pos": [27.323597,7.68083,27.528622], "name": "Lambda Serpentis"}, {"pos": [15.18033,4.9715166,27.097368], "name": "Gliese 569"}, {"pos": [84.628105,10.796772,26.215439], "name": "HD 153631"}, {"pos": [9.53116,-26.496164,10.644481], "name": "HD 102365"}, {"pos": [13.368647,-14.92643,19.414978], "name": "61 Virginis"}, {"pos": [67.25486,13.409554,32.501785], "name": "GJ 3969"}, {"pos": [35.03779,6.0090923,31.989937], "name": "Psi Serpentis"}, {"pos": [12.639404,3.402629,34.30819], "name": "Arcturus"}, {"pos": [10.80381,1.0012312,35.558548], "name": "Eta Bootis"}, {"pos": [51.67138,26.666878,50.675377], "name": "49 Serpentis"}, {"pos": [0.15711975,-17.413523,31.105112], "name": "Beta Virginis"}, {"pos": [8.596655,-16.152271,33.4903], "name": "Porrima"}, {"pos": [69.101105,0.008838654,38.985416], "name": "Xi Scorpii B"}, {"pos": [52.05275,8.68693,45.191547], "name": "HD 141272"}, {"pos": [40.42933,-0.710454,41.077644], "name": "HD 135204"}, {"pos": [10.012228,-22.182964,30.990334], "name": "Ross 948"}, {"pos": [71.655045,-0.08240509,40.317394], "name": "Xi Scorpii A"}, {"pos": [52.982117,-5.0580463,40.596077], "name": "Gliese 586"}, {"pos": [51.597195,13.07421,51.50213], "name": "Alpha Serpentis"}, {"pos": [14.056,-0.29752922,48.844646], "name": "Tau Bootis"}, {"pos": [39.224777,-9.335512,44.021736], "name": "Mu Virginis"}, {"pos": [30.499287,-6.982547,46.450413], "name": "HD 126053"}, {"pos": [21.918701,-2.9222603,50.763496], "name": "HD 122742"}, {"pos": [17.162552,-13.426081,45.487152], "name": "HD 116442"}, {"pos": [18.033545,-13.995666,47.626476], "name": "HD 116443"}, {"pos": [55.560333,-19.879847,46.20997], "name": "Alpha Librae"}, {"pos": [56.912003,3.6440115,59.99234], "name": "5 Serpentis"}, {"pos": [39.98794,-14.6784,53.116806], "name": "GJ 544A"}, {"pos": [55.38301,-22.86063,48.887665], "name": "HIP 71743"}, {"pos": [42.866615,-17.155485,56.910217], "name": "Iota Virginis"}, {"pos": [80.45487,-10.083662,61.824787], "name": "Epsilon Librae"}, {"pos": [60.596138,10.997223,82.4464], "name": "HD 134066"}, {"pos": [52.170826,6.379628,80.483215], "name": "HD 131023"}, {"pos": [77.63544,-12.087709,73.83237], "name": "HD 133352"}, {"pos": [76.01227,-11.389217,78.32384], "name": "HD 132375"}, {"pos": [66.93439,5.585,91.82704], "name": "HD 132307"}]';
    const obj = JSON.parse(json_str);
    for (let i = 0; i < obj.length; i++) {
        const element = obj[i];
        let mesh =new THREE.Mesh( new THREE.SphereGeometry( star_radius, 8, 8 ), material );

        const moonDiv = document.createElement( 'div' );
        moonDiv.className = 'label';
        moonDiv.textContent = element["name"];
        moonDiv.style.marginTop = '-0.5em';
        const moonLabel = new CSS2DObject( moonDiv );
        moonLabel.position.set( up_vec.x*star_radius, up_vec.y*star_radius, up_vec.z*star_radius );
        mesh.add( moonLabel );
        mesh.position.set( element["pos"][0], element["pos"][1], element["pos"][2] );
        scene.add( mesh );
    }

    // lines
    const line_points = [];
    let center = new THREE.Vector3(0, 0, 0);
    add_ring(line_points, center, 10.0);
    add_ring(line_points, center, 20.0);
    add_ring(line_points, center, 30.0);
    add_ring(line_points, center, 40.0);
    add_ring(line_points, center, 50.0);
    const line_geometry = new THREE.BufferGeometry().setFromPoints( line_points );
    const line = new THREE.LineSegments( line_geometry, new THREE.LineBasicMaterial({color: 0x808080}) );
    scene.add( line );

    
    renderer.setSize( container.clientWidth, container.clientHeight );
    container.appendChild( renderer.domElement );
    
    
    labelRenderer.setSize( container.clientWidth, container.clientHeight );
    labelRenderer.domElement.style.position = 'absolute';
    labelRenderer.domElement.style.top = '0px';
    container.appendChild( labelRenderer.domElement );

    const controls = new OrbitControls( camera, labelRenderer.domElement  );
    controls.minDistance = 20;
    controls.maxDistance = 150;

    function animate() {
        requestAnimationFrame( animate );
        renderer.render( scene, camera );
        labelRenderer.render( scene, camera );
    }
    animate();
    window.addEventListener( 'resize', onWindowResize, false );
}

function onWindowResize() {
    camera.aspect = window.innerWidth / window.innerHeight;
    camera.updateProjectionMatrix();

    renderer.setSize( window.innerWidth, window.innerHeight );
    labelRenderer.setSize( window.innerWidth, window.innerHeight );
}



export {init};