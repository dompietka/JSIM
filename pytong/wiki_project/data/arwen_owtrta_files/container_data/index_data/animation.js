// call Adlib.screenshotterEnd() on the last animation code.
// do not delete initAnimation function since this is the first function that will be called after initialization of defaultValues.
// if ever there is a video for this ad, you can use myVideo as the variable to play the video.
let tween, preloadImgs=[]

function initAnimation() {
     // place all fluid elements before text resize and css attrib.
     gsap.to(this, 1, {
          onStart:()=>{ 
               initLayout();
               Adlib.templateCSS(defaultValues.cssStyle); // DO NOT DELETE THIS
               // Static-PreRendered (Convert px to rem for screenshotter)
               if(defaultValues.trigger3 === "Static-PreRendered") {
                    remConverter(["font-size"],["#titleText","#heroText","#cta","#secondaryText","#copyright","#legal span"]);
                    remConverter(["padding"],["#ctaContainer"]);
                    remConverter(["margin-top"],["#hboMaxLogo","#stackLogo","#dynamicLogoContainer","#ctaContainer","#secondaryText"]);
               }
          },onComplete:()=>{ 
               preloadImages(preloadImgs)
                    .then(() => startAnimation())
                    .catch((error) => console.error('Error loading images:', error));
          }
     });
}

function startAnimation() {  
     document.querySelector("#mainContent").style.opacity = 1;
     // Add 5 seconds delay before screenshots
     setTimeout(() => { takeScreenshot(); animationEnd(); }, 5000);
}

function animationEnd() {
     setTimeout(function() {adlibEnd();},1000);
}

function initLayout() {
     // Auto-Styling
     function autoStyle(...args) {
          args.forEach(([element, value, isIcon = false]) => {
               const elem = document.querySelector(element),
                     styleCondition = isIcon ? /(\[ICON-|\[BOLD_WHITE-|\[BOLD_BLUE-)/i.test(value) : /(\[BOLD_WHITE-|\[BOLD_BLUE-)/i.test(value);
               if (styleCondition) {
                    let cleanedOutput = value;
                    cleanedOutput = cleanedOutput.replace(/\[ICON-(#(?:[a-fA-F0-9]{6}))-(.*?)\]/g, '<live style="background-color: $1;">$2</live>')
                                                 .replace(/\[ICON-(.*?)\]/gi, '<live>$1</live>')
                                                 .replace(/\[BOLD_WHITE-(.*?)\]/gi, '<white>$1</white>')
                                                 .replace(/\[BOLD_BLUE-(.*?)\]/gi, '<blue>$1</blue>');
                    elem.innerHTML = cleanedOutput;
               }
          });
     }
     autoStyle(
          ["#titleText", defaultValues.frame1Subheadline],
          ["#heroText", defaultValues.frame1Headline, true],
          ["#secondaryText", defaultValues.frame1Headline2]
     );

     // Logo Triggers
     if(!checkBlankImage("subLogo")) {
          gsap.set("#hboMaxLogo", {display:"none"});
          gsap.set("#dynamicLogoContainer", {display:"block"});
     } else {
          if(defaultValues.trigger2 === "Max DAZN") {
               stackLogo.src = "hboDAZNCenterLogo.svg";
               gsap.set("#hboMaxLogo", {display:"none"});
               gsap.set("#stackLogo",{display:"block", width:175});
          }
          if(defaultValues.trigger2 === "Max Iridescent") {
               stackLogo.src = "hboIridescentLogo.svg";
               gsap.set("#hboMaxLogo", {display:"none"});
               gsap.set("#stackLogo",{display:"block", width:101});
          }
          if(defaultValues.trigger2 === "Home of the Olympics") {
               stackLogo.src = "hboHOTOCenterLogo.svg";
               gsap.set("#hboMaxLogo", {display:"none"});
               gsap.set("#stackLogo",{display:"block", width:175});
          }
     }

     // Full Bleed Image
     if(defaultValues.trigger === "Full Screen/Native") {
          gsap.set("#fullBleedBG",{display:"block"});
          gsap.set(["#keyartContainer","#bottomGrad","#copyrightContainer","#stackContainer","#legalContainer"], {display:"none"});
     }

     // Gradients
     const topRGB = hexToRgb("uiElement1", defaultValues.uiElement1);
     const bottomRGB = hexToRgb("uiElement3", defaultValues.uiElement3);
     gsap.set("#topGrad", {background:`linear-gradient(0deg, rgba(${topRGB}, 0.00) 0%, rgba(${topRGB}, 0.00) 6.25%, rgba(${topRGB}, 0.02) 12.5%, rgba(${topRGB}, 0.04) 18.75%, rgba(${topRGB}, 0.06) 25%, rgba(${topRGB}, 0.10) 31.25%, rgba(${topRGB}, 0.14) 37.5%, rgba(${topRGB}, 0.19) 43.75%, rgba(${topRGB}, 0.25) 50%, rgba(${topRGB}, 0.32) 56.25%, rgba(${topRGB}, 0.39) 62.5%, rgba(${topRGB}, 0.47) 68.75%, rgba(${topRGB}, 0.56) 75%, rgba(${topRGB}, 0.66) 81.25%, rgba(${topRGB}, 0.77) 87.5%, rgba(${topRGB}, 0.88) 93.75%, rgba(${topRGB}, 1) 100%)`});
     gsap.set("#bottomGrad", {background:`linear-gradient(180deg, rgba(${bottomRGB}, 0.00) 0%, rgba(${bottomRGB}, 0.00) 6.25%, rgba(${bottomRGB}, 0.02) 12.5%, rgba(${bottomRGB}, 0.04) 18.75%, rgba(${bottomRGB}, 0.06) 25%, rgba(${bottomRGB}, 0.10) 31.25%, rgba(${bottomRGB}, 0.14) 37.5%, rgba(${bottomRGB}, 0.19) 43.75%, rgba(${bottomRGB}, 0.25) 50%, rgba(${bottomRGB}, 0.32) 56.25%, rgba(${bottomRGB}, 0.39) 62.5%, rgba(${bottomRGB}, 0.47) 68.75%, rgba(${bottomRGB}, 0.56) 75%, rgba(${bottomRGB}, 0.66) 81.25%, rgba(${bottomRGB}, 0.77) 87.5%, rgba(${bottomRGB}, 0.88) 93.75%, rgba(${bottomRGB}, 1) 100%)`});
     gsap.set("#topGrad", {opacity:parseFloat(defaultValues.uiElement2)});
     gsap.set("#bottomGrad", {opacity:parseFloat(defaultValues.uiElement4)});
     
     // CTA + BG + HBO MAX color 
     const ctaBoxColorOpacity = hexToRgb("ctaBoxColor" , defaultValues.ctaBoxColor) + `, ${defaultValues.uiElement5})`;
     gsap.set("#ctaContainer", {background:`rgba(${ctaBoxColorOpacity}`});
     gsap.set("#frame1", {background:defaultValues.bgColor});
     updateSvgFill("hboMaxLogo", defaultValues.frame1ShapeColor);

     Adlib.textResize();

     // Legal Stack Adjustment
     let legalHeight = Adlib.isEmpty(defaultValues.legal) ? 0 : Adlib.getStyle(document.querySelector("#legal span"), "height");
     gsap.set("#stackContainer",{height: Adlib.getStyle(document.getElementById("stackContainer"), "height") - legalHeight});
     
     // Conditionally hide elements
     const headline1OFF = Adlib.isEmpty(defaultValues.frame1Headline);
     const headline2OFF = Adlib.isEmpty(defaultValues.frame1Headline2);
     if(headline1OFF) {
          gsap.set("#heroTextContainer", {display:"none"});
          gsap.set("#stackContainer", {height:164, top:408});
          noCtaLayout("headline1OFF");
     }
     if(headline2OFF) {
          gsap.set("#secondaryText", {display:"none"});
          gsap.set("#stackContainer", {top:404});
          noCtaLayout("headline2OFF");
     }
     if(headline1OFF && headline2OFF) {
          gsap.set("#stackContainer", {height:162, top:409});
          noCtaLayout("anyStringGoesHere", "headline2OFF");
     }
     noCtaLayout();
     if(Adlib.isEmpty(defaultValues.frame1Subheadline)) gsap.set("#titleText",{display:"none"});
     if(checkBlankImage("frame1Image")) gsap.set("#cornerLogo",{display:"none"});
     if(Adlib.isEmpty(defaultValues.legal)) {
          gsap.set("#legalContainer",{display:"none"})  
          // gsap.set("#stackContainer", {top:423});  
     }
}

function noCtaLayout(elementOFF, secondElementOFF = null) {
     if(Adlib.isEmpty(defaultValues.ctaText)) {
          gsap.set("#ctaContainer", {display:"none"});
          // First check if both headlines are off
          if (secondElementOFF !== null) {
               gsap.set("#stackContainer", {top:408});
               return;
          }
          // Only one of the headlines is off
          if(elementOFF === "headline1OFF") {
               gsap.set("#stackContainer", {top:413});
          } else if(elementOFF === "headline2OFF") {
               gsap.set("#stackContainer", {top:405});
          }
     }
}

function remConverter(styles, elems) {
     // Convert to rem based on 16
     function convertValueToRem(value) { return isNaN(parseFloat(value)) ? value : parseFloat(value) / 16 + 'rem'; }
     // Multiple values checker
     function convertValuesToRem(values) { return values.map(value => convertValueToRem(value)).join(' '); }
     // Get values
     elems.forEach(elem => {
          if(document.querySelector(elem)) {
               const computedStyles = getComputedStyle(document.querySelector(elem));
               styles.forEach(property => { document.querySelector(elem).style[property] = convertValuesToRem(computedStyles[property].split(' ')); });
          }
     });
}

function checkBlankImage(img) {
     // Blob blank image fix for Producer, false = not blank
     if(defaultValues[img].indexOf("blank") !== -1 || 
     defaultValues[img] === "" || 
     defaultValues[img] === null || 
     defaultValues[img] === undefined || 
     (document.querySelector(`img[adlib-image="${img}"]`).naturalWidth === 1 && 
     document.querySelector(`img[adlib-image="${img}"]`).naturalHeight === 1)) 
     return true;
     else return false;
}

function hexToRgb(defaultValue, hex) {
     // Remove # and make lowercase
     hex = hex.trim().replace(/^#/, "").toLowerCase();
     // Expand shorthand form ("03F") to full form ("0033FF")
     if (hex.length === 3) {
          hex = hex.split("").map(char => char + char).join("");
     }
     // Wrong hex will throw warning and return fallback (black)
     if (!/^([a-f\d]{6})$/i.test(hex)) {
          console.warn(`Invalid ${defaultValue} hex color: ${hex}. Defaulting to black.`);
          return "0, 0, 0";
     }
     const r = parseInt(hex.substring(0, 2), 16);
     const g = parseInt(hex.substring(2, 4), 16);
     const b = parseInt(hex.substring(4, 6), 16);
     return `${r}, ${g}, ${b}`;
}

function updateSvgFill(svgId, color) {
     const svg = document.getElementById(svgId);
     if (!svg) return;
     // Select all paths with fill attribute
     const paths = svg.querySelectorAll("path[fill]");
     paths.forEach(path => {
          path.setAttribute("fill", color);
     });
}

// Preload images
function preloadImages(urls, maxRetries = 5) {
     const imageUrls = removeDuplicatesAndEmptyStrings(urls);
     const promises = imageUrls.map(url => loadImages(url, maxRetries));
     return Promise.all(promises);
}
function loadImages(url, maxRetries) {
    let retryCount = 0;
    function load() {
        return new Promise((resolve, reject) => {
            const img = new Image();
            img.onload = () => resolve(url);
            img.onerror = () => reject(url);
            img.src = url;
        }).catch(error => {
            if (retryCount < maxRetries) {
                console.log(`Retrying ${url}...`);
                retryCount++;
                return load(); // Retry loading
            } else {
                throw new Error(`Failed to load image after ${maxRetries} retries: ${url}`);
            }
        });
    }
    return load();
}
function removeDuplicatesAndEmptyStrings(array) {
     return array.filter(function(item, index) { return item && array.indexOf(item) === index; });
}



