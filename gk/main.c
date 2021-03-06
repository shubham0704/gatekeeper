/*
 * Gatekeeper - DoS protection system.
 * Copyright (C) 2016 Digirati LTDA.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <rte_log.h>
#include <rte_lcore.h>
#include <rte_ethdev.h>

#include "gatekeeper_gk.h"
#include "gatekeeper_main.h"
#include "gatekeeper_config.h"

/*
 * Define the custom log type for GK functional block,
 * which is used to generate logs for GK block.
 */
#define RTE_LOGTYPE_GK RTE_LOGTYPE_USER1

#define GATEKEEPER_MAX_PKT_BURST (32)

static int
gk_proc(__attribute__((unused)) void *arg)
{
	/* TODO Implement the basic algorithm of a GK block. */

	uint32_t lcore = rte_lcore_id();

	RTE_LOG(NOTICE, GK, "The GK block is running at lcore = %u\n", lcore);

	while (likely(!exiting)) {
		/* 
		 * XXX Sample setting for test only.
		 * 
		 * Here, just use two ports (0, 1) and 1 queue (0) for test.
		 *
		 * Port and queue identifiers should be changed 
		 * according to configuration.
		 */

		/* Get burst of RX packets, from first port of pair. */
		uint16_t num_rx;
		uint16_t num_tx;
		struct rte_mbuf *bufs[GATEKEEPER_MAX_PKT_BURST];

		num_rx = rte_eth_rx_burst(0, 0, bufs, GATEKEEPER_MAX_PKT_BURST);

		if (unlikely(num_rx == 0))
			continue;

		/* Send burst of TX packets, to second port of pair. */
		num_tx = rte_eth_tx_burst(1, 0, bufs, num_rx);

		/* Free any unsent packets. */
		if (unlikely(num_tx < num_rx)) {
			int i;
			for (i = num_tx; i < num_rx; i++)
				rte_pktmbuf_free(bufs[i]);
		}
	}

	RTE_LOG(NOTICE, GK, "The GK block at lcore = %u is exiting\n", lcore);

	return 0;
}

int
run_gk(const struct gk_config *gk_conf)
{
	/* TODO Initialize and run GK functional block. */

	unsigned int i;
	int ret;
	for (i = gk_conf->lcore_start_id; i <= gk_conf->lcore_end_id; i++) {
		ret = rte_eal_remote_launch(gk_proc, NULL, i);
		if (ret) {
			RTE_LOG(ERR, EAL, "lcore %u failed to launch GK\n", i);
			return ret;
		}
	}

	return 0;
}
