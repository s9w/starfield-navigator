import * as THREE from 'three';
import { CSS2DRenderer, CSS2DObject } from 'three/addons/renderers/CSS2DRenderer.js';
import { OrbitControls } from 'three/addons/controls/OrbitControls.js';


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
let camera = new THREE.PerspectiveCamera( 90, container.clientWidth / container.clientHeight, 0.1, 1000 );
let renderer = new THREE.WebGLRenderer({antialias: true});
let labelRenderer = new CSS2DRenderer();
let scene = new THREE.Scene();
let graph = new WeightedGraph();
let position_lookup = Object();

const up_vec = new THREE.Vector3(-0.484225601, 0.746894360, 0.455712944);
const right_vec = new THREE.Vector3(0.0548099577, -0.493931174, 0.867771626);
const front_vec = new THREE.Vector3(0.873337090, 0.445001483, 0.198131084);

let rings_obj;
let connections_obj;
let path_obj;

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


function update_path()
{
    scene.remove(path_obj);
    let line_points = [];
    
    const line_geometry = new THREE.BufferGeometry().setFromPoints( line_points );
    path_obj = new THREE.LineSegments( line_geometry, new THREE.LineBasicMaterial({color: 0x3b7b3b}) );
    scene.add( path_obj );
}


function range_changed(new_range)
{
    update_connections(new_range);
    
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
            let v0 = new THREE.Vector3(json_data[i]["pos"][0], json_data[i]["pos"][1], json_data[i]["pos"][2]); 
            let v1 = new THREE.Vector3(json_data[j]["pos"][0], json_data[j]["pos"][1], json_data[j]["pos"][2]); 
            let dist = v0.distanceTo(v1);
            if(dist > new_range)
            continue;
            graph.addEdge(json_data[i]["name"], json_data[j]["name"], dist);
        }
    }
    let jump_graph = graph.Dijkstra("Sol", "Porrima");
    for (let i = 0; i < jump_graph.length-1; i++) {
        let pos0 = position_lookup[jump_graph[i]];
        let pos1 = position_lookup[jump_graph[i+1]];
        let dist = pos0.distanceTo(pos1);
        console.log("%s to %s: %f", jump_graph[i], jump_graph[i+1], dist);
    }
}

function update_connections(new_range)
{
    scene.remove(connections_obj);
    let line_points = [];
    for (let i = 0; i < json_data.length; i++)
    {
        for (let j = 0; j < json_data.length; j++)
        {
            if(i==j)
            continue;
            let v0 = new THREE.Vector3(json_data[i]["pos"][0], json_data[i]["pos"][1], json_data[i]["pos"][2]); 
            let v1 = new THREE.Vector3(json_data[j]["pos"][0], json_data[j]["pos"][1], json_data[j]["pos"][2]); 
            let dist = v0.distanceTo(v1);
            if(dist > new_range)
            continue;
            
            line_points.push(new THREE.Vector3(json_data[i]["pos"][0], json_data[i]["pos"][1], json_data[i]["pos"][2]));
            line_points.push(new THREE.Vector3(json_data[j]["pos"][0], json_data[j]["pos"][1], json_data[j]["pos"][2]));
        }
    }
    
    const line_geometry = new THREE.BufferGeometry().setFromPoints( line_points );
    connections_obj = new THREE.LineSegments( line_geometry, new THREE.LineBasicMaterial({color: 0x3b7b3b}) );
    scene.add( connections_obj );
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
        moonDiv.className = 'label';
        moonDiv.textContent = json_data[i]["name"];
        moonDiv.style.marginTop = '-0.5em';
        const moonLabel = new CSS2DObject( moonDiv );
        moonLabel.position.set( up_vec.x*star_radius, up_vec.y*star_radius, up_vec.z*star_radius );
        mesh.add( moonLabel );
        mesh.position.set( json_data[i]["pos"][0], json_data[i]["pos"][1], json_data[i]["pos"][2] );
        scene.add( mesh );
        
        let option_el = document.createElement("option");
        option_el.text = json_data[i]["name"];
        option_el.value = json_data[i]["name"];
        document.getElementById('from-select').appendChild(option_el.cloneNode(true));
        document.getElementById('to-select').appendChild(option_el.cloneNode(true));
        
        position_lookup[json_data[i]["name"]] = new THREE.Vector3(json_data[i]["pos"][0], json_data[i]["pos"][1], json_data[i]["pos"][2]);
    }
    
    update_rings(new THREE.Vector3(0, 0, 0));
    update_connections(document.querySelector("#jump_range").value);
    
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
