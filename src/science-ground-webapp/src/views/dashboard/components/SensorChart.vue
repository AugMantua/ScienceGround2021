<template>
  <v-card elevation="0" outlined>
    <apexchart type="line" :options="options" :series="series"></apexchart>
  </v-card>
</template>

<script>
import VueApexCharts from "vue-apexcharts";
import Vue from "vue";

export default {
  name: "SensorChart",
  components: {
    apexchart: VueApexCharts,
  },
  props: ["sensorDatas", "terrariumId"],
  data() {
    return {
      options: {
        chart: {
          id: "vuechart-example",
        },
        xaxis: {
          labels: {
            show: false,
          },
        },
      },
      series: [
        {
          name: "series-1",
          data: [],
        },
      ],
    };
  },

  mounted() {
    let self = this;
    self.getTerrariunDatas();
  },

  methods: {
    getTerrariunDatas() {
      let self = this;

      let data = {
        TerrariumId: self.terrariumId,
        From: "2010",
        To: "2021",
        SensorId: self.sensorDatas.SensorID,
      };

      Vue.axios
        .post("/data/measures/get", data)
        .then((res) => {
          res.data.forEach((element) => {
            self.series[0].data.push({
              x: element.Timestamp,
              y: element.Value,
            });
          });
        })
        .catch((err) => {
          console.log(err);
        });
    },
  },
};
</script>