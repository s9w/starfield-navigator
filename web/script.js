import * as THREE from 'three';
import { CSS2DRenderer, CSS2DObject } from 'three/addons/renderers/CSS2DRenderer.js';
import { OrbitControls } from 'three/addons/controls/OrbitControls.js';

let container = document.getElementById('glContainer');
let camera = new THREE.PerspectiveCamera( 90, container.clientWidth / container.clientHeight, 0.1, 1000 );
let renderer = new THREE.WebGLRenderer({antialias: true});
let labelRenderer = new CSS2DRenderer();
let scene = new THREE.Scene();

const up_vec = new THREE.Vector3(-0.484225601, 0.746894360, 0.455712944);
const right_vec = new THREE.Vector3(0.0548099577, -0.493931174, 0.867771626);
const front_vec = new THREE.Vector3(0.873337090, 0.445001483, 0.198131084);

let rings_obj;

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

function update_rings(center)
{
    scene.remove(rings_obj);
    let line_points = [];
    add_ring(line_points, center, 10.0);
    add_ring(line_points, center, 20.0);
    add_ring(line_points, center, 30.0);
    add_ring(line_points, center, 40.0);
    add_ring(line_points, center, 50.0);
    const line_geometry = new THREE.BufferGeometry().setFromPoints( line_points );
    rings_obj = new THREE.LineSegments( line_geometry, new THREE.LineBasicMaterial({color: 0x808080}) );
    scene.add( rings_obj );
}

function init() {
    camera.up = up_vec;
    camera.position.add(front_vec);
    camera.position.multiplyScalar(-20.0);
    camera.lookAt(0, 0, 0);

    const material = new THREE.MeshBasicMaterial( { color: 0xffffff } );
    // scene.add( new THREE.Mesh( new THREE.SphereGeometry( 0.2, 32, 16 ).translate(1, 0, 0), new THREE.MeshBasicMaterial( { color: 0xff0000 } ) ) );
    // scene.add( new THREE.Mesh( new THREE.SphereGeometry( 0.2, 32, 16 ).translate(0, 1, 0), new THREE.MeshBasicMaterial( { color: 0x00ff00 } ) ) );
    // scene.add( new THREE.Mesh( new THREE.SphereGeometry( 0.2, 32, 16 ).translate(0, 0, 1), new THREE.MeshBasicMaterial( { color: 0x0000ff } ) ) );

    const star_radius = 0.5;

    for (let i = 0; i < json_data.length; i++) {
        const element = json_data[i];
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

        let option_el = document.createElement("option");
        option_el.text = element["name"];
        option_el.value = element["name"];
        document.getElementById('from-select').appendChild(option_el.cloneNode(true));
        document.getElementById('to-select').appendChild(option_el.cloneNode(true));
    }

    update_rings(new THREE.Vector3(0, 0, 0));

    
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