var template = {
  version: "1.0.0",
  width: 300,
  height: 600,
  elements: {
    frame1Subheadline: {
      name: "Title Text",
      reportingDimension: "frame1Subheadline",
      text: { value: "LOREM IPSUM DOLOR SIT AMET, AD HAS PONDERUM U" },
      style: { content: "normal" },
      maxLength: 45
    },
    frame1Image: {
      name: "Attrib Logo",
      reportingDimension: "frame1Image",
      image: { src: "maxOriginal.png" },
      width: 150,
      height: 58
    },
    uiElement1: {
      name: "Gradient Color Top",
      reportingDimension: "uiElement1",
      text: { value: "#000000" }
    },
    uiElement2: {
      name: "Opacity Control Top",
      reportingDimension: "uiElement2",
      text: { value: "1.0" }
    },
    uiElement3: {
      name: "Gradient Color Bot",
      reportingDimension: "uiElement3",
      text: { value: "#000000" }
    },
    uiElement4: {
      name: "Opacity Control Bot",
      reportingDimension: "uiElement4",
      text: { value: "1.0" }
    },
    frame1Image2: {
      name: "Keyart",
      reportingDimension: "frame1Image2",
      image: { src: "keyart.png" },
      variableSize: {
        variableElementId:"trigger",
          sizes: {
            "Display":{
              width: 713, 
              height: 1275,
            },
            "Amazon":{
              width: 4751, 
              height: 8500,
            }
          }
      }
    },
    frame1Headline: {
      name: "Main Copy",
      reportingDimension: "frame1Headline",
      text: { value: "[ICON-#F60301-LIVE] LOREM IPSUM DOLOR SIT AMET, AD HAS PONDERUM URBANITAS, HAS POSTULANT" },
      style: { content: "normal" },
      maxLength: 72
    },
    frame1Headline2: {
      name: "Sub Copy",
      reportingDimension: "frame1Headline2",
      text: { value: "LOREM IPSUM DOLOR SIT AMETS" },
      style: { content: "normal" },
      maxLength: 27
    },
    frame1Headline3: {
      name: "Copyright",
      reportingDimension: "frame1Headline3",
      text: { value: "Lorem ipsum dolor sit amet consectetur adipiscing elit sed do eiusmod tempor incididunt ut labore et dolore magna aliqua ut enim" },
      style: { content: "normal" },
      maxLength: 128
    },
    ctaText: {
      name: "CTA Text",
      reportingDimension: "ctaText",
      text: { value: "LOREM IPSUM DOLOR SIT AM" },
      style: { content: "normal" },
      maxLength: 24
    },
    ctaBoxColor: {
      name: "CTA Color",
      reportingDimension: "ctaBoxColor",
      text: { value: "#8298AB" }
    },
    uiElement5: {
      name: "CTA Opacity",
      reportingDimension: "uiElement5",
      text: { value: "0.36" }
    },
    frame1ShapeColor: {
      name: "Logo Color",
      reportingDimension: "frame1ShapeColor",
      text: { value: "#FFFFFF" }
    },
    legal: {
      name: "Legal",
      reportingDimension: "legal",
      text: { value: "Lorem ipsum dolor sit amet, ad has ponderum urbanitas, has postulant maiestatis no. Soluta populo ocurreret ne qui. Ad audiam ma" },
      style: { content: "normal" },
      maxLength: 250
    },
    baseImage: {
      name: "Full Bleed Image",
      reportingDimension: "baseImage",
      image: { src: "fullbleed.png" },
    },
    subLogo: {
      name: "Dynamic Logo",
      image: { src: "blank.png" },
      width: 782,
      height: 278
    },
    trigger: {
      name: "Category Trigger",
      reportingDimension: "trigger",
      text: { value:"Display" },
      enum: [
        { label: "Display", value: "Display" },
        { label: "Amazon", value: "Amazon" },
        { label: "Full Screen/Native", value: "Full Screen/Native" }
      ]
    },
    trigger2: {
      name: "Logo Trigger",
      reportingDimension: "trigger2",
      text: { value:"HBO Max" },
      enum: [
        { label: "HBO Max", value: "HBO Max" },
        { label: "Max DAZN", value: "Max DAZN" },
        { label: "Max Iridescent", value: "Max Iridescent" },
        { label: "Home of the Olympics", value: "Home of the Olympics" },
      ]
    },
    trigger3: {
      name: "Delivery Trigger",
      reportingDimension: "trigger3",
      text: { value:"Static-Dynamic" },
      enum: [
        { label: "Static-Dynamic", value: "Static-Dynamic" },
        { label: "Static-PreRendered", value: "Static-PreRendered" }
      ]
    },
    bgColor: {
      name: "Background Color",
      reportingDimension: "bgColor",
      text: { value: "#000000" }
    },
    customVariable: {
      name: "Custom Variable 1",
      reportingDimension: "customVariable",
      text: { value: "claravine" }
    },
    customVariable2: {
      name: "Custom Variable 2",
      reportingDimension: "customVariable2",
      text: { value: "title name" }
    },
    cssStyle: {
      name: "CSS Style",
      text: { value: "/* CSS Styling */" }
    },
    landingPage: {
      name: "Landing Page",
      reportingDimension: "landingPage",
      url: { value: "https://www.max.com/" }
    },
  }
};

var runtime = new TemplateRuntime(template).start();