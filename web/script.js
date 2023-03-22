import * as THREE from 'three';
import { CSS2DRenderer, CSS2DObject } from 'three/addons/renderers/CSS2DRenderer.js';
import { OrbitControls } from 'three/addons/controls/OrbitControls.js';
import { LineGeometry } from 'three/addons/lines/LineGeometry.js';
import { LineMaterial } from 'three/addons/lines/LineMaterial.js';
import { Line2 } from 'three/addons/lines/Line2.js';
import { LineSegments2 } from 'three/addons/lines/LineSegments2.js';


//helper class for PriorityQueue
class Node {
    constructor(val, priority) {
        this.val = val;
        this.priority = priority;
    }
}

class PriorityQueue {
    constructor() {
        this.values = [];
    }
    enqueue(val, priority) {
        let newNode = new Node(val, priority);
        this.values.push(newNode);
        this.bubbleUp();
    }
    bubbleUp() {
        let idx = this.values.length - 1;
        const element = this.values[idx];
        while (idx > 0) {
            let parentIdx = Math.floor((idx - 1) / 2);
            let parent = this.values[parentIdx];
            if (element.priority >= parent.priority) break;
            this.values[parentIdx] = element;
            this.values[idx] = parent;
            idx = parentIdx;
        }
    }
    dequeue() {
        const min = this.values[0];
        const end = this.values.pop();
        if (this.values.length > 0) {
            this.values[0] = end;
            this.sinkDown();
        }
        return min;
    }
    sinkDown() {
        let idx = 0;
        const length = this.values.length;
        const element = this.values[0];
        while (true) {
            let leftChildIdx = 2 * idx + 1;
            let rightChildIdx = 2 * idx + 2;
            let leftChild, rightChild;
            let swap = null;
            
            if (leftChildIdx < length) {
                leftChild = this.values[leftChildIdx];
                if (leftChild.priority < element.priority) {
                    swap = leftChildIdx;
                }
            }
            if (rightChildIdx < length) {
                rightChild = this.values[rightChildIdx];
                if (
                    (swap === null && rightChild.priority < element.priority) ||
                    (swap !== null && rightChild.priority < leftChild.priority)
                    ) {
                        swap = rightChildIdx;
                    }
                }
                if (swap === null) break;
                this.values[idx] = this.values[swap];
                this.values[swap] = element;
                idx = swap;
            }
        }
    }
    
    //Dijkstra's algorithm only works on a weighted graph.
    
    class WeightedGraph {
        constructor() {
            this.adjacencyList = {};
        }
        addVertex(vertex) {
            if (!this.adjacencyList[vertex]) this.adjacencyList[vertex] = [];
        }
        addEdge(vertex1, vertex2, weight) {
            this.adjacencyList[vertex1].push({ node: vertex2, weight });
            this.adjacencyList[vertex2].push({ node: vertex1, weight });
        }
        Dijkstra(start, finish) {
            const nodes = new PriorityQueue();
            const distances = {};
            const previous = {};
            let path = []; //to return at end
            let smallest;
            //build up initial state
            for (let vertex in this.adjacencyList) {
                if (vertex === start) {
                    distances[vertex] = 0;
                    nodes.enqueue(vertex, 0);
                } else {
                    distances[vertex] = Infinity;
                    nodes.enqueue(vertex, Infinity);
                }
                previous[vertex] = null;
            }
            // as long as there is something to visit
            while (nodes.values.length) {
                smallest = nodes.dequeue().val;
                if (smallest === finish) {
                    //WE ARE DONE
                    //BUILD UP PATH TO RETURN AT END
                    while (previous[smallest]) {
                        path.push(smallest);
                        smallest = previous[smallest];
                    }
                    break;
                }
                if (smallest || distances[smallest] !== Infinity) {
                    for (let neighbor in this.adjacencyList[smallest]) {
                        //find neighboring node
                        let nextNode = this.adjacencyList[smallest][neighbor];
                        //calculate new distance to neighboring node
                        let candidate = distances[smallest] + nextNode.weight;
                        let nextNeighbor = nextNode.node;
                        if (candidate < distances[nextNeighbor]) {
                            //updating new smallest distance to neighbor
                            distances[nextNeighbor] = candidate;
                            //updating previous - How we got to neighbor
                            previous[nextNeighbor] = smallest;
                            //enqueue in priority queue with new priority
                            nodes.enqueue(nextNeighbor, candidate);
                        }
                    }
                }
            }
            return path.concat(smallest).reverse();
        }
    }
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
let container = document.getElementById('glContainer');
let camera = new THREE.PerspectiveCamera( 80, container.clientWidth / container.clientHeight, 0.1, 1000 );
let renderer = new THREE.WebGLRenderer({antialias: true});
let labelRenderer = new CSS2DRenderer();
let controls;
let scene = new THREE.Scene();
let graph = new WeightedGraph();
let position_lookup = Object();
let mode = "orbit";

const up_vec = new THREE.Vector3(-0.484225601, 0.746894360, 0.455712944);
const right_vec = new THREE.Vector3(0.0548099577, -0.493931174, 0.867771626);
const front_vec = new THREE.Vector3(0.873337090, 0.445001483, 0.198131084);

let connections_group = new THREE.Group();
let path_group = new THREE.Group();
let ring_group = new THREE.Group();
scene.add( ring_group );
scene.add( connections_group );
scene.add( path_group );

function add_ring_point(center, angle, radius, line_points)
{
    let point = center.clone();
    let front_shift = front_vec.clone();
    front_shift.multiplyScalar(radius * Math.cos(angle));
    point.add(front_shift);
    let right_shift = right_vec.clone();
    right_shift.multiplyScalar(radius * Math.sin(angle));
    point.add(right_shift);
    line_points.push(point.x, point.y, point.z);
}


function add_ring(center, radius)
{
    const n = 32;
    let line_points = [];
    for (let i = 0; i < n; i++)
    {
        const angle = 1.0*i/(n-1)*2.0*Math.PI;
        add_ring_point(center, angle, radius, line_points);
    }
    let line_geometry = new LineGeometry();
    line_geometry.setPositions( line_points );
    let rings_obj = new Line2( line_geometry, new LineMaterial({color: 0x808080, linewidth: 0.002}) );
    ring_group.add(rings_obj)
    
}


function update_rings(center)
{
    ring_group.clear();

    add_ring(center, 10.0);
    add_ring(center, 20.0);
    add_ring(center, 30.0);
    add_ring(center, 40.0);
    add_ring(center, 50.0);
}


function update_path()
{
    path_group.clear();

    let jump_graph = graph.Dijkstra(document.getElementById('from').innerHTML, document.getElementById('to').innerHTML);

    if(jump_graph.length == 1)
        document.querySelector('#no_path').style = "";
    else
        document.querySelector('#no_path').style = "visibility: hidden;";

    for (let i = 0; i < jump_graph.length-1; i++) {
        let line_points = [];
        
        let pos0 = position_lookup[jump_graph[i]];
        let pos1 = position_lookup[jump_graph[i+1]];
        line_points.push(pos0.x, pos0.y, pos0.z);
        line_points.push(pos1.x, pos1.y, pos1.z);
        // let dist = pos0.distanceTo(pos1);

        let line_geometry = new LineGeometry();
        line_geometry.setPositions( line_points );
        let path_obj = new Line2( line_geometry, new LineMaterial({color: 0xff2020, linewidth: 0.005}) );
        path_group.add(path_obj)
    }
}


function range_changed(new_range)
{
    graph = new WeightedGraph();
    for (let i = 0; i < json_data.length; i++)
    {
        graph.addVertex(json_data[i]["name"]);
    }
    for (let i = 0; i < json_data.length; i++)
    {
        for (let j = 0; j < json_data.length; j++)
        {
            if(i==j)
                continue;
            let dx = json_data[i]["pos"][0] - json_data[j]["pos"][0];
            let dy = json_data[i]["pos"][1] - json_data[j]["pos"][1];
            let dz = json_data[i]["pos"][2] - json_data[j]["pos"][2];
            let dist2 = dx*dx + dy*dy + dz*dz;
            if(dist2 > (new_range*new_range))
                continue;
            graph.addEdge(json_data[i]["name"], json_data[j]["name"], Math.sqrt(dist2));
        }
    }

    update_connections(new_range);
    update_path();
}


function update_connections(new_range)
{
    connections_group.clear();
    
    const mat = new LineMaterial({color: 0x3b7b3b, linewidth: 0.001});
    for (let i = 0; i < json_data.length; i++)
    {
        for (let j = 0; j < json_data.length; j++)
        {
            if(i==j)
                continue;

            let dx = json_data[i]["pos"][0] - json_data[j]["pos"][0];
            let dy = json_data[i]["pos"][1] - json_data[j]["pos"][1];
            let dz = json_data[i]["pos"][2] - json_data[j]["pos"][2];
            let dist2 = dx*dx + dy*dy + dz*dz;
            if(dist2 > (new_range*new_range))
                continue;
            
            let line_points = [];
            line_points.push(json_data[i]["pos"][0], json_data[i]["pos"][1], json_data[i]["pos"][2]);
            line_points.push(json_data[j]["pos"][0], json_data[j]["pos"][1], json_data[j]["pos"][2]);
            let line_geometry = new LineGeometry();
            line_geometry.setPositions( line_points );
            let rings_obj = new Line2( line_geometry, mat );
            connections_group.add(rings_obj)
        }
    }
}


function on_mode_change(e)
{
    mode = e.target.value;
    if(mode == "select_from")
        controls.enabled = false;
    if(mode == "select_to")
        controls.enabled = false;
    if(mode == "orbit")
        controls.enabled = true;
}


function on_label_click(name)
{
    if(mode == "orbit")
        return;
    if(mode == "select_from")
        document.getElementById('from').innerHTML = name;
    if(mode == "select_to")
        document.getElementById('to').innerHTML = name;

    update_path();
}


function init() {
    camera.up = up_vec;
    camera.position.add(front_vec);
    camera.position.multiplyScalar(-20.0);
    camera.lookAt(0, 0, 0);
    document.querySelector("#jump_range").addEventListener("change", (event) => {
        range_changed(event.target.value);
    });
    
    
    const material = new THREE.MeshBasicMaterial( { color: 0xffffff } );
    // scene.add( new THREE.Mesh( new THREE.SphereGeometry( 0.2, 32, 16 ).translate(1, 0, 0), new THREE.MeshBasicMaterial( { color: 0xff0000 } ) ) );
    // scene.add( new THREE.Mesh( new THREE.SphereGeometry( 0.2, 32, 16 ).translate(0, 1, 0), new THREE.MeshBasicMaterial( { color: 0x00ff00 } ) ) );
    // scene.add( new THREE.Mesh( new THREE.SphereGeometry( 0.2, 32, 16 ).translate(0, 0, 1), new THREE.MeshBasicMaterial( { color: 0x0000ff } ) ) );
    
    const star_radius = 0.5;
    
    for (let i = 0; i < json_data.length; i++) {
        let mesh =new THREE.Mesh( new THREE.SphereGeometry( star_radius, 8, 8 ), material );
        
        const moonDiv = document.createElement( 'div' );
        moonDiv.addEventListener('click', function(){on_label_click(json_data[i]["name"])} );

        moonDiv.className = 'label';
        moonDiv.textContent = json_data[i]["name"];
        moonDiv.style.marginTop = '-0.5em';
        const moonLabel = new CSS2DObject( moonDiv );
        moonLabel.position.set( up_vec.x*star_radius, up_vec.y*star_radius, up_vec.z*star_radius );
        mesh.add( moonLabel );
        mesh.position.set( json_data[i]["pos"][0], json_data[i]["pos"][1], json_data[i]["pos"][2] );
        scene.add( mesh );
        
        position_lookup[json_data[i]["name"]] = new THREE.Vector3(json_data[i]["pos"][0], json_data[i]["pos"][1], json_data[i]["pos"][2]);
    }
    
    range_changed(document.getElementById("jump_range").value);
    update_rings(new THREE.Vector3(0, 0, 0));
    // update_connections(document.querySelector("#jump_range").value);
    
    labelRenderer.setSize( container.clientWidth, container.clientHeight );
    labelRenderer.domElement.style.position = 'absolute';
    labelRenderer.domElement.style.top = '0px';
    container.appendChild( labelRenderer.domElement );

    renderer.setSize( container.clientWidth, container.clientHeight );
    container.appendChild( renderer.domElement );
    
    controls = new OrbitControls( camera, labelRenderer.domElement  );
    controls.minDistance = 20;
    controls.maxDistance = 150;
    
    function animate() {
        requestAnimationFrame( animate );
        renderer.render( scene, camera );
        labelRenderer.render( scene, camera );
    }
    animate();
    window.addEventListener( 'resize', onWindowResize, false );

    document.getElementById('orbit').addEventListener( 'change', on_mode_change );
    document.getElementById('select_from').addEventListener( 'change', on_mode_change );
    document.getElementById('select_to').addEventListener( 'change', on_mode_change );
}


function onWindowResize() {
    camera.aspect = window.innerWidth / window.innerHeight;
    camera.updateProjectionMatrix();
    
    renderer.setSize( window.innerWidth, window.innerHeight );
    labelRenderer.setSize( window.innerWidth, window.innerHeight );
}


export {init};
