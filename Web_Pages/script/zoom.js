
let zoomArr = [0.5,0.75,0.85,0.9,1,1.2,1.5];


var element = document.querySelector('.boot_plugins');
let value = element.getBoundingClientRect().width / element.offsetWidth;

let indexofArr = 4;
handleChange = ()=>{
  let val = document.querySelector('#sel').value; 
  val = Number(val)
  console.log('handle change selected value ',val);
  indexofArr = zoomArr.indexOf(val);
 console.log('Handle changes',indexofArr)
  element.style['transform'] = `scale(${val})`
}



document.querySelector('.zoomin').addEventListener('click',()=>{
  console.log('value of index zoomin is',indexofArr)
  if(indexofArr < zoomArr.length-1){
    indexofArr += 1;
    value = zoomArr[indexofArr];
    document.querySelector('#sel').value = value
    // console.log('current value is',value)
    // console.log('scale value is',value)
    element.style['transform'] = `scale(${value})`
  }
})

document.querySelector('.zoomout').addEventListener('click',()=>{
 console.log('value of index  zoom out is',indexofArr)
  if(indexofArr >0){
     indexofArr -= 1;
     value = zoomArr[indexofArr];
     document.querySelector('#sel').value = value
  // console.log('scale value is',value)
  element.style['transform'] = `scale(${value})`
  }
})