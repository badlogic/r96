const canvas = document.querySelector("#output");
const ctx = canvas.getContext("2d");
const pixelSize = 100 * devicePixelRatio;
const subPixelSize = 10 * devicePixelRatio;
const realLineWidth = 2;
let pixelColor = "#00f";
const circleColor = "#f00";
const rectColor = "#0f0";
let p1 = { x: 1.2, y: 2.5 };
let p2 = { x: 2.9, y: 2.9 };
const steppedPoints = [];
const helperLines = [];
let algorithm = lineIntegerDDA;

function drawGrid(gridSize, color) {
   ctx.strokeStyle = color;
   for (let y = 0; y < canvas.height; y += gridSize) {
      for (let x = 0; x < canvas.width; x += gridSize) {
         ctx.beginPath();
         ctx.rect(x, y, gridSize, gridSize);
         ctx.stroke();
      }
   }
}

function drawPixelCenters(gridSize, color) {
   ctx.strokeStyle = color;
   for (let y = gridSize / 2; y < canvas.height; y += gridSize) {
      for (let x = gridSize / 2; x < canvas.width; x += gridSize) {
         ctx.beginPath();
         ctx.moveTo(x - gridSize / 10, y);
         ctx.lineTo(x + gridSize / 10, y);
         ctx.moveTo(x, y - gridSize / 10);
         ctx.lineTo(x, y + gridSize / 10);
         ctx.stroke();

         ctx.beginPath();
         ctx.moveTo(x - gridSize / 2, y);
         ctx.lineTo(x, y - gridSize / 2);
         ctx.lineTo(x + gridSize / 2, y);
         ctx.lineTo(x, y + gridSize / 2);
         ctx.lineTo(x - gridSize / 2, y);
         ctx.stroke();
      }
   }
}

function drawLine(x1, y1, x2, y2, color) {
   ctx.strokeStyle = color;
   ctx.beginPath();
   ctx.moveTo(x1 * pixelSize, y1 * pixelSize);
   ctx.lineTo(x2 * pixelSize, y2 * pixelSize);
   ctx.stroke();
}

function drawCircle(x, y, radius) {
   ctx.strokeStyle = circleColor;
   ctx.beginPath();
   ctx.arc(x * pixelSize, y * pixelSize, radius * pixelSize, 0, 2 * Math.PI);
   ctx.stroke();
}

function drawRect(x, y, width, height, color) {
   if (!color) color = rectColor;
   ctx.fillStyle = color;
   ctx.beginPath();
   ctx.rect(x * pixelSize, y * pixelSize, width * pixelSize, height * pixelSize);
   ctx.fill();
}

function drawPixel(x, y) {
   ctx.fillStyle = pixelColor;
   ctx.beginPath();
   ctx.rect(x * pixelSize, y * pixelSize, pixelSize, pixelSize);
   ctx.fill();
}

function lineIntegerDDA(x1, y1, x2, y2) {
   x1 = Math.floor(x1);
   y1 = Math.floor(y1);
   x2 = Math.floor(x2);
   y2 = Math.floor(y2);
   const deltaX = (x2 - x1);
   const deltaY = (y2 - y1);

   if (deltaX == 0 && deltaY == 0) {
      drawPixel(x1, y1);
      return;
   }

   const numPixels = Math.abs(deltaX) > Math.abs(deltaY) ? Math.abs(deltaX) : Math.abs(deltaY);
   const stepX = deltaX / numPixels;
   const stepY = deltaY / numPixels;
   let x = x1 + 0.5;
   let y = y1 + 0.5;
   for (let i = 0; i <= numPixels; i++) {
      steppedPoints.push({ x: x, y: y });
      drawPixel(Math.floor(x), Math.floor(y));
      x += stepX;
      y += stepY;
   }
}

function lineFloatDDA(x1, y1, x2, y2) {
   const deltaX = (x2 - x1);
   const deltaY = (y2 - y1);

   const numPixelsX = Math.abs(Math.floor(x2) - Math.floor(x1)) + 1;
   const numPixelsY = Math.abs(Math.floor(y2) - Math.floor(y1)) + 1;
   const numPixels = numPixelsX > numPixelsY ? numPixelsX : numPixelsY;

   if (numPixels == 1) {
      drawPixel(Math.floor(x1), Math.floor(y1));
      return;
   }

   const stepX = deltaX / (numPixels - 1);
   const stepY = deltaY / (numPixels - 1);
   let x = x1;
   let y = y1;
   for (let i = 0; i < numPixels; i++) {
      steppedPoints.push({ x: x, y: y });
      drawPixel(Math.floor(x), Math.floor(y));
      x += stepX;
      y += stepY;
   }
}

// Fails:
// p1 = {x: 5.821875, y: 5.15}
// p2 = {x: 9.840625, y: 9.65625}
function lineFloatDDACenterSampling(x1, y1, x2, y2) {
   const deltaX = (x2 - x1);
   const deltaY = (y2 - y1);

   const numPixelsX = Math.abs(Math.floor(x2) - Math.floor(x1)) + 1;
   const numPixelsY = Math.abs(Math.floor(y2) - Math.floor(y1)) + 1;
   let numPixels = Math.abs(deltaX) > Math.abs(deltaY) ? numPixelsX : numPixelsY;

   if (numPixels == 1) {
      drawPixel(Math.floor(x1), Math.floor(y1));
      return;
   }

   let x, y, stepX, stepY;
   if (Math.abs(deltaX) > Math.abs(deltaY)) {
      stepX = x1 < x2 ? 1 : -1;
      stepY = deltaY / Math.abs(deltaX);
      x = Math.floor(x1) + 0.5 + stepX;
      y = y1 + Math.abs(x - x1) * stepY;
   } else {
      stepY = y1 < y2 ? 1 : -1;
      stepX = deltaX / Math.abs(deltaY);
      y = Math.floor(y1) + 0.5 + stepY;
      x = x1 + Math.abs(y - y1) * stepX;
   }

   drawPixel(Math.floor(x1), Math.floor(y1));
   for (let i = 1; i < numPixels; i++) {
      steppedPoints.push({ x: x, y: y });
      drawPixel(Math.floor(x), Math.floor(y));
      x += stepX;
      y += stepY;
   }
}

function lineFloatDDACenterSamplingIncludeEndpoint(x1, y1, x2, y2) {
   const deltaX = (x2 - x1);
   const deltaY = (y2 - y1);

   const numPixelsX = Math.abs(Math.floor(x2) - Math.floor(x1)) + 1;
   const numPixelsY = Math.abs(Math.floor(y2) - Math.floor(y1)) + 1;
   let numPixels = Math.abs(deltaX) > Math.abs(deltaY) ? numPixelsX : numPixelsY;

   if (numPixels == 1) {
      drawPixel(Math.floor(x1), Math.floor(y1));
      return;
   }

   let x, y, stepX, stepY;
   if (Math.abs(deltaX) > Math.abs(deltaY)) {
      stepX = x1 < x2 ? 1 : -1;
      stepY = deltaY / Math.abs(deltaX);
      x = Math.floor(x1) + 0.5 + stepX;
      y = y1 + Math.abs(x - x1) * stepY;
   } else {
      stepY = y1 < y2 ? 1 : -1;
      stepX = deltaX / Math.abs(deltaY);
      y = Math.floor(y1) + 0.5 + stepY;
      x = x1 + Math.abs(y - y1) * stepX;
   }

   drawPixel(Math.floor(x1), Math.floor(y1));
   for (let i = 1; i < numPixels - 1; i++) {
      steppedPoints.push({ x: x, y: y });
      drawPixel(Math.floor(x), Math.floor(y));
      x += stepX;
      y += stepY;
   }

   // Last calculated pixel coordinate != end point pixel coordinate?
   // Plot both calculated pixel and end point pixel.
   // E.g.
   // p1 = {x: 5.49375, y: 5.478125}
   // p2 = {x: 10.121875, y: 10.890625}
   if (Math.floor(x2) != Math.floor(x) || Math.floor(y2) != Math.floor(y)) {
      // Check if the calculated sub-pixel coordinate is outside
      // the line segment. E.g. 
      // p1 = {x: 5.49375, y: 5.478125}
      // p2 = {x: 1.1999999999999993, y: 10.21875}
      steppedPoints.push({ x: x, y: y });
      drawPixel(Math.floor(x), Math.floor(y));
   }
   drawPixel(Math.floor(x2), Math.floor(y2));
}

/*function lineIntegerBresenham(x1, y1, x2, y2) {
   x1 = Math.floor(x1);
   y1 = Math.floor(y1);
   x2 = Math.floor(x2);
   y2 = Math.floor(y2);
   var deltaX = Math.abs(x2 - x1);
   var deltaY = Math.abs(y2 - y1);
   var stepX = (x1 < x2) ? 1 : -1;
   var stepY = (y1 < y2) ? 1 : -1;
   var err = deltaX - deltaY;

   while (true) {
      steppedPoints.push({ x: x1 + 0.5, y: y1 + 0.5 });
      drawPixel(x1, y1);

      if ((x1 === x2) && (y1 === y2)) break;
      var e2 = 2 * err;
      if (e2 > -deltaY) { err -= deltaY; x1 += stepX; }
      if (e2 < deltaX) { err += deltaX; y1 += stepY; }
   }
}*/

//
// Basic Bresenham for all octants. `x` and `y` are
// set to the start point coordinates. `error` starts out
// storing the the (absolute) distance from the point on
// line sampled at the next coordinate along the major axis to the pixel
// border "above" it. If that distance is > 0.5, we move
// "up" by 1 on the minor axis, and decrement the error by 1.
//
function lineIntegerBresenham1(x1, y1, x2, y2) {
   x1 = Math.floor(x1);
   y1 = Math.floor(y1);
   x2 = Math.floor(x2);
   y2 = Math.floor(y2);
   let deltaX = Math.abs(x2 - x1);
   let deltaY = Math.abs(y2 - y1);
   let stepX = (x1 < x2) ? 1 : -1;
   let stepY = (y1 < y2) ? 1 : -1;
   let x = x1;
   let y = y1;

   if (deltaX == 0 && deltaY == 0) {
      drawPixel(x, y);
      return;
   }

   if (deltaX >= deltaY) {
      let slope = deltaY / deltaX;
      let error = slope;
      for (let i = 0, n = deltaX; i <= n; i++) {
         steppedPoints.push({ x: x + 0.5, y: y + 0.5 });
         drawPixel(x, y);
         if (error > 0.5) {
            error -= 1;
            y += stepY;
         }
         x += stepX;
         error += slope;
      }
   } else {
      let slope = deltaX / deltaY;
      let error = slope;
      for (let i = 0, n = deltaY; i <= n; i++) {
         steppedPoints.push({ x: x + 0.5, y: y + 0.5 });
         drawPixel(x, y);
         if (error > 0.5) {
            error -= 1;
            x += stepX;
         }
         y += stepY;
         error += slope;
      }
   }
   if (x != Math.floor(x2) && y != Math.floor(y2)) {
      drawPixel(x, y);
   }
}

//
// Rewrites the "move up" condition from error > 0.5 to error > 0
// by subtracting 0.5 from the initial error.
//
function lineIntegerBresenham2(x1, y1, x2, y2) {
   x1 = Math.floor(x1);
   y1 = Math.floor(y1);
   x2 = Math.floor(x2);
   y2 = Math.floor(y2);
   let deltaX = Math.abs(x2 - x1);
   let deltaY = Math.abs(y2 - y1);
   let stepX = (x1 < x2) ? 1 : -1;
   let stepY = (y1 < y2) ? 1 : -1;
   let x = x1;
   let y = y1;

   if (deltaX == 0 && deltaY == 0) {
      drawPixel(x, y);
      return;
   }

   if (deltaX >= deltaY) {
      let slope = deltaY / deltaX;
      let error = slope - 0.5;
      for (let i = 0, n = deltaX; i <= n; i++) {
         steppedPoints.push({ x: x + 0.5, y: y + 0.5 });
         drawPixel(x, y);
         if (error > 0) {
            error -= 1;
            y += stepY;
         }
         x += stepX;
         error += slope;
      }
   } else {
      let slope = deltaX / deltaY;
      let error = slope - 0.5;
      for (let i = 0, n = deltaY; i <= n; i++) {
         steppedPoints.push({ x: x + 0.5, y: y + 0.5 });
         drawPixel(x, y);
         if (error > 0) {
            error -= 1;
            x += stepX;
         }
         y += stepY;
         error += slope;
      }
   }
}

//
// Gets rid of the division to calculate the slope by
// multiplying the right hand side of assignments to
// error by deltaX or deltaY, depending on the major axis.
// This also eliminates `slope` everywhere.
function lineIntegerBresenham3(x1, y1, x2, y2) {
   x1 = Math.floor(x1);
   y1 = Math.floor(y1);
   x2 = Math.floor(x2);
   y2 = Math.floor(y2);
   let deltaX = Math.abs(x2 - x1);
   let deltaY = Math.abs(y2 - y1);
   let stepX = (x1 < x2) ? 1 : -1;
   let stepY = (y1 < y2) ? 1 : -1;
   let x = x1;
   let y = y1;

   if (deltaX == 0 && deltaY == 0) {
      drawPixel(x, y);
      return;
   }

   if (deltaX >= deltaY) {
      let error = deltaY - 0.5 * deltaX;
      for (let i = 0, n = deltaX; i <= n; i++) {
         steppedPoints.push({ x: x + 0.5, y: y + 0.5 });
         drawPixel(x, y);
         if (error > 0) {
            error -= deltaX;
            y += stepY;
         }
         x += stepX;
         error += deltaY;
      }
   } else {
      let error = deltaX - 0.5 * deltaY;
      for (let i = 0, n = deltaY; i <= n; i++) {
         steppedPoints.push({ x: x + 0.5, y: y + 0.5 });
         drawPixel(x, y);
         if (error > 0) {
            error -= deltaY;
            x += stepX;
         }
         y += stepY;
         error += deltaX;
      }
   }
}


// Multiplies the right hand side of all assignments to error
// by 2 to remove the multiplication by 0.5 when calculating
// the initial error. This makes the algorithm integer only.
function lineIntegerBresenham(x1, y1, x2, y2) {
   x1 = Math.floor(x1);
   y1 = Math.floor(y1);
   x2 = Math.floor(x2);
   y2 = Math.floor(y2);
   let deltaX = Math.abs(x2 - x1);
   let deltaY = Math.abs(y2 - y1);
   let stepX = (x1 < x2) ? 1 : -1;
   let stepY = (y1 < y2) ? 1 : -1;
   let x = x1;
   let y = y1;

   if (deltaX == 0 && deltaY == 0) {
      drawPixel(x, y);
      return;
   }

   if (deltaX >= deltaY) {
      let error = 2 * deltaY - deltaX;
      for (let i = 0, n = deltaX; i <= n; i++) {
         steppedPoints.push({ x: x + 0.5, y: y + 0.5 });
         drawPixel(x, y);
         if (error > 0) {
            error -= deltaX * 2;
            y += stepY;
         }
         x += stepX;
         error += deltaY * 2;
      }
   } else {
      let error = 2 * deltaX - deltaY;
      for (let i = 0, n = deltaY; i <= n; i++) {
         steppedPoints.push({ x: x + 0.5, y: y + 0.5 });
         drawPixel(x, y);
         if (error > 0) {
            error -= deltaY * 2;
            x += stepX;
         }
         y += stepY;
         error += deltaX * 2;
      }
   }
}

// 
// Makes Bresenham1 sub-pixel aware by initializing the error
// accordingly.
// 
// Foobar intersection calculation, fix it
/*function lineSubPixelBresenham1(x1, y1, x2, y2) {
   let deltaX = Math.abs(x2 - x1);
   let deltaY = Math.abs(y2 - y1);
   let stepX = (x1 < x2) ? 1 : -1;
   let stepY = (y1 < y2) ? 1 : -1;
   let x = Math.floor(x1);
   let y = Math.floor(y1);

   if (deltaX == 0 && deltaY == 0) {
      drawPixel(x, y);
      return;
   }

   if (deltaX >= deltaY) {
      // The start point position is no longer at
      // (0.5,0.5) but somewhere within [0,0]-(1,1)
      // The error for the next point is thus not the slope
      // but the slope
      let slope = deltaY / deltaX;
      let ix = x + 0.5 + stepX;
      let dx = ix - x1;
      let iy = y1 + dx * (y2 - y1) / (x2 - x1);
      let dy = (y + 0.5) - iy;
      let error = y2 < y1 ? dy : -dy;
      for (let i = 0, n = Math.abs(Math.floor(x2) - x); i <= n; i++) {
         steppedPoints.push({ x: x + 0.5, y: y + 0.5 });
         helperLines.push({ x1: x + 0.5, y1: y + 0.5, x2: x + 0.5, y2: y + 0.5 + (y2 < y1 ? -error : error) });
         drawPixel(x, y);
         if (error > 0.5) {
            error -= 1;
            y += stepY;
         }
         x += stepX;
         error += slope;
      }
   } else {
      let slope = deltaX / deltaY;
      let iy = y + 0.5 + stepY;
      let dy = iy - y1;
      let ix = x1 + dy * (x2 - x1) / (y2 - y1);
      let dx = (x + 0.5) - ix;
      let error = x2 < x1 ? dx : -dx;
      for (let i = 0, n = Math.abs(Math.floor(y2) - y); i <= n; i++) {
         steppedPoints.push({ x: x + 0.5, y: y + 0.5 });
         helperLines.push({ x1: x + 0.5, y1: y + 0.5, x2: x + 0.5 + (x2 < x1 ? -error : error), y2: y + 0.5 });
         drawPixel(x, y);
         if (error > 0.5) {
            error -= 1;
            x += stepX;
         }
         y += stepY;
         error += slope;
      }
   }
}*/

function lineSubPixelBresenham0(x1, y1, x2, y2) {
   let deltaX = Math.abs(x2 - x1);
   let deltaY = Math.abs(y2 - y1);
   let stepX = (x1 < x2) ? 1 : -1;
   let stepY = (y1 < y2) ? 1 : -1;
   let x = Math.floor(x1);
   let y = Math.floor(y1);

   if (deltaX == 0 && deltaY == 0) {
      drawPixel(x, y);
      return;
   }

   if (deltaX >= deltaY) {
      let dist_next_pixel = Math.abs(Math.floor(x1) + 0.5 + stepX - x1);
      let dist_pixel_edge = Math.abs(y1) - Math.floor(Math.abs(y1));
      if (y1 > y2)
         dist_pixel_edge = 1 - dist_pixel_edge;
      let error = dist_pixel_edge + dist_next_pixel * deltaY / deltaX;
      let numPixels = Math.abs(Math.floor(x2) - Math.floor(x));
      for (let i = 0; i <= numPixels; i++) {
         steppedPoints.push({ x: x + 0.5, y: y + 0.5 });
         helperLines.push({ x1: x + 0.5, y1: y + (y2 < y1 ? -stepY : 0), x2: x + 0.5, y2: y + (y2 < y1 ? -error : error) + (y2 < y1 ? -stepY : 0) });
         drawPixel(x, y);
         if (error >= 1) {
            error -= 1;
            y += stepY;
         }
         x += stepX;
         error += deltaY / deltaX;
      }
   } else {
      let dist_next_pixel = Math.abs(Math.floor(y1) + 0.5 + stepY - y1);
      let dist_pixel_edge = Math.abs(x1) - Math.floor(Math.abs(x1));
      if (x1 > x2)
         dist_pixel_edge = 1 - dist_pixel_edge;
      let error = dist_pixel_edge + dist_next_pixel * deltaX / deltaY;
      let numPixels = Math.abs(Math.floor(y2) - Math.floor(y));
      for (let i = 0; i <= numPixels; i++) {
         steppedPoints.push({ x: x + 0.5, y: y + 0.5 });
         helperLines.push({ x1: x + (x2 < x1 ? -stepX : 0), y1: y + 0.5, x2: x + (x2 < x1 ? -error : error) + (x2 < x1 ? -stepX : 0), y2: y + 0.5 });

         drawPixel(x, y);
         if (error >= 1) {
            error -= 1;
            x += stepX;
         }
         y += stepY;
         error += deltaX / deltaY;
      }
   }
}

function lineSubPixelBresenham1(x1, y1, x2, y2) {
   let deltaX = Math.abs(x2 - x1);
   let deltaY = Math.abs(y2 - y1);
   let stepX = (x1 < x2) ? 1 : -1;
   let stepY = (y1 < y2) ? 1 : -1;
   let x = Math.floor(x1);
   let y = Math.floor(y1);

   // if (Math.abs(Math.floor(x2) - Math.floor(x1)) >= Math.abs(Math.floor(y2) - Math.floor(y1))) {
   if (deltaX + 1 >= deltaY) {
      let dist_next_pixel = Math.abs(Math.floor(x1) + 0.5 + stepX - x1);
      let dist_pixel_edge = Math.abs(y1) - Math.floor(Math.abs(y1));
      if (y1 > y2) dist_pixel_edge = 1 - dist_pixel_edge;
      let error = dist_pixel_edge * deltaX + dist_next_pixel * deltaY;
      let numPixels = Math.abs(Math.floor(x2) - Math.floor(x));
      for (let i = 0; i < numPixels; i++) {
         steppedPoints.push({ x: x + 0.5, y: y + 0.5 });
         helperLines.push({ x1: x + 0.5, y1: y + (y2 < y1 ? -stepY : 0), x2: x + 0.5, y2: y + (y2 < y1 ? -error / deltaX : error / deltaX) + (y2 < y1 ? -stepY : 0) });
         drawPixel(x, y);
         if (error >= deltaX) {
            error -= deltaX;
            y += stepY;
         }
         x += stepX;
         error += deltaY;
      }
   } else {
      let dist_next_pixel = Math.abs(Math.floor(y1) + 0.5 + stepY - y1);
      let dist_pixel_edge = Math.abs(x1) - Math.floor(Math.abs(x1));
      if (x1 > x2) dist_pixel_edge = 1 - dist_pixel_edge;
      let error = dist_pixel_edge * deltaY + dist_next_pixel * deltaX;
      let numPixels = Math.abs(Math.floor(y2) - Math.floor(y));
      for (let i = 0; i < numPixels; i++) {
         steppedPoints.push({ x: x + 0.5, y: y + 0.5 });
         helperLines.push({ x1: x + (x2 < x1 ? -stepX : 0), y1: y + 0.5, x2: x + (x2 < x1 ? -error / deltaY : error / deltaY) + (x2 < x1 ? -stepX : 0), y2: y + 0.5 });

         drawPixel(x, y);
         if (error >= deltaY) {
            error -= deltaY;
            x += stepX;
         }
         y += stepY;
         error += deltaX;
      }
   }

   // Fix case where last drawn pixel does not contain
   // end point but line enters and exits diamond.
   // p1 = { x: 4.101875, y: 2.140000000000001 }
   // p2 = { x: 1.8106249999999995, y: 4.766250000000001 }
   //
   // This might result in overshooting, e.g.
   // p1 = {x: 4.101875, y: 2.140000000000001}
   // p2 = {x: 1.9206249999999994, y: 3.876250000000001}
   //
   // That also needs fixing...
   if (x != Math.floor(x2) || y != Math.floor(y2)) {
      drawPixel(x, y);
   }
}

function main() {
   canvas.width = canvas.clientWidth * devicePixelRatio;
   canvas.height = canvas.clientHeight * devicePixelRatio;

   console.innerHTML = "";
   steppedPoints.length = 0;
   helperLines.length = 0;
   pixelColor = "#00f";
   algorithm(p1.x, p1.y, p2.x, p2.y);
   // pixelColor = "#0f0";
   // algorithm(p3.x, p3.y, p4.x, p4.y);

   // drawGrid(subPixelSize, "#555");
   drawGrid(pixelSize, "#ccc");
   drawPixelCenters(pixelSize, "#0c0");

   drawLine(p1.x, p1.y, p2.x, p2.y, "rgb(254, 121, 5)");
   drawRect(p1.x - 1 / subPixelSize / 2, p1.y - 1 / subPixelSize / 2, 1 / subPixelSize, 1 / subPixelSize, "#f00");
   drawRect(p2.x - 1 / subPixelSize / 2, p2.y - 1 / subPixelSize / 2, 1 / subPixelSize, 1 / subPixelSize, "#ff0");

   for (let i = 0; i < helperLines.length; i++) {
      let line = helperLines[i];
      drawLine(line.x1, line.y1, line.x2, line.y2, "#ff0");
   }
   for (let i = 0; i < steppedPoints.length; i++) {
      const point = steppedPoints[i];
      drawRect(point.x - 1 / subPixelSize / 2, point.y - 1 / subPixelSize / 2, 1 / subPixelSize, 1 / subPixelSize);
   }

   if (closestPoint) drawCircle(closestPoint.x, closestPoint.y, 0.25);
}

let dragStart = { x: 0, y: 0 };
let draggedPoint = null;
let closestPoint = null;

function down(gridX, gridY) {
   let distP1 = Math.sqrt((gridX - p1.x) * (gridX - p1.x) + (gridY - p1.y) * (gridY - p1.y));
   let distP2 = Math.sqrt((gridX - p2.x) * (gridX - p2.x) + (gridY - p2.y) * (gridY - p2.y));
   if (Math.min(distP1, distP2) > 0.25) {
      draggedPoint = null;
      return;
   }
   closestPoint = draggedPoint = distP1 < distP2 ? p1 : p2;
   dragStart.x = gridX;
   dragStart.y = gridY;
   requestAnimationFrame(main);
}

function move(gridX, gridY) {
   let distP1 = Math.sqrt((gridX - p1.x) * (gridX - p1.x) + (gridY - p1.y) * (gridY - p1.y));
   let distP2 = Math.sqrt((gridX - p2.x) * (gridX - p2.x) + (gridY - p2.y) * (gridY - p2.y));

   if (Math.min(distP1, distP2) > 0.25) {
      closestPoint = null;
   } else {
      closestPoint = distP1 < distP2 ? p1 : p2;
   }

   if (draggedPoint) {
      let deltaX = gridX - dragStart.x;
      let deltaY = gridY - dragStart.y;
      draggedPoint.x += deltaX;
      draggedPoint.y += deltaY;
      dragStart.x = gridX;
      dragStart.y = gridY;
      localStorage.setItem("points", JSON.stringify({ p1: p1, p2: p2 }));
   }
   requestAnimationFrame(main);
}

canvas.addEventListener("mousedown", function (mouseEvent) {
   let gridX = mouseEvent.offsetX * devicePixelRatio / pixelSize;
   let gridY = mouseEvent.offsetY * devicePixelRatio / pixelSize;
   down(gridX, gridY);
}, false);

canvas.addEventListener("touchstart", function (touchEvent) {
   touchEvent.preventDefault();
   const touch = touchEvent.targetTouches[0];
   let rect = touchEvent.target.getBoundingClientRect();
   let x = touch.pageX - rect.left;
   let y = touch.pageY - rect.top;
   let gridX = x * devicePixelRatio / pixelSize;
   let gridY = y * devicePixelRatio / pixelSize;
   down(gridX, gridY);
})

canvas.addEventListener("mousemove", function (mouseEvent) {
   let gridX = mouseEvent.offsetX * devicePixelRatio / pixelSize;
   let gridY = mouseEvent.offsetY * devicePixelRatio / pixelSize;
   move(gridX, gridY);
});

canvas.addEventListener("touchmove", function (touchEvent) {
   touchEvent.preventDefault();
   const touch = touchEvent.targetTouches[0];
   let rect = touchEvent.target.getBoundingClientRect();
   let x = touch.pageX - rect.left;
   let y = touch.pageY - rect.top;
   let gridX = x * devicePixelRatio / pixelSize;
   let gridY = y * devicePixelRatio / pixelSize;
   move(gridX, gridY);
})

canvas.addEventListener("mouseup", function (mouseEvent) {
   draggedPoint = null;
}, false);

canvas.addEventListener("mouseleave", function () {
   draggedPoint = null;
   closestPoint = null;
})

function selectAlgorithm(name) {
   if (name == "lineIntegerDDA") algorithm = lineIntegerDDA;
   else if (name == "lineIntegerBresenham") algorithm = lineIntegerBresenham;
   else if (name == "lineSubPixelBresenham1") algorithm = lineSubPixelBresenham1;
   else if (name == "lineFloatDDA") algorithm = lineFloatDDA;
   else if (name == "lineFloatDDACenterSampling") algorithm = lineFloatDDACenterSampling;
   else if (name == "lineFloatDDACenterSamplingIncludeEndpoint") algorithm = lineFloatDDACenterSamplingIncludeEndpoint;
   else algorithm = lineIntegerDDA;
   requestAnimationFrame(main);
}

const algorithmSelect = document.querySelector("#algorithm");
algorithmSelect.addEventListener("change", () => {
   selectAlgorithm(algorithmSelect.value);
   localStorage.setItem("algorithm", algorithmSelect.value);
});

const snapButton = document.querySelector("#snap");
snapButton.addEventListener("click", () => {
   p1.x = Math.floor(p1.x) + 0.5;
   p1.y = Math.floor(p1.y) + 0.5;
   p2.x = Math.floor(p2.x) + 0.5;
   p2.y = Math.floor(p2.y) + 0.5;
   localStorage.setItem("points", JSON.stringify({ p1: p1, p2: p2 }));
   requestAnimationFrame(main);
});

const resetButton = document.querySelector("#reset");
resetButton.addEventListener("click", () => {
   p1 = { x: 1.2, y: 2.5 };
   p2 = { x: 2.9, y: 2.9 };
   localStorage.setItem("points", JSON.stringify({ p1: p1, p2: p2 }));
   requestAnimationFrame(main);
})

const swapButton = document.querySelector("#swap");
swapButton.addEventListener("click", () => {
   let tmp = p2;
   p2 = p1;
   p1 = tmp;
   localStorage.setItem("points", JSON.stringify({ p1: p1, p2: p2 }));
   requestAnimationFrame(main);
})

if (localStorage.getItem("points")) {
   let points = JSON.parse(localStorage.getItem("points"));
   p1 = points.p1;
   p2 = points.p2;
}
// p1 = { x: 5.821875 - 4, y: 5.15 - 4 }
// p2 = { x: 9.840625 - 4, y: 9.65625 - 4 }

// gap
// p1 = { x: 4.101875, y: 2.140000000000001 }
//p2 = { x: 1.8106249999999995, y: 4.766250000000001 }

// Overshooting
p1 = { x: 4.101875, y: 2.140000000000001 }
p2 = { x: 1.9206249999999994, y: 3.876250000000001 }

// Tom failure case
// p1 = { x: 198.046173 - 198, y: 167.552826 - 167 }; p2 = { x: 204.918884 - 198, y: 169.901337 - 167 };
// let p3 = { x: 204.918884 - 198, y: 169.901337 - 167 }; p4 = { x: 208.969208 - 198, y: 165.241364 - 167 };

if (localStorage.getItem("algorithm")) {
   let algorithm = localStorage.getItem("algorithm");
   algorithmSelect.value = algorithm;
   selectAlgorithm(algorithm);
} else {
   selectAlgorithm(algorithmSelect.value);
}

requestAnimationFrame(main);