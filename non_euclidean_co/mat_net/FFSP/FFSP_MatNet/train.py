
"""
The MIT License

Copyright (c) 2021 MatNet

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.



THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
"""

##########################################################################################
# Machine Environment Config

DEBUG_MODE = False
USE_CUDA = not DEBUG_MODE
CUDA_DEVICE_NUM = 0

JOB_CNT = 50
USE_POMO = False
POMO_SIZE = 12
SEED = 1234
DISTIL_COEFF = 0.001
NO_SYMMETRIC = False

if USE_POMO:
    POMO_SIZE = 24  # 4!


##########################################################################################
# Path Config

import os
import sys

os.chdir(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, "..")  # for problem_def
sys.path.insert(0, "../..")  # for utils


##########################################################################################
# import

import logging

from utils.utils import create_logger, copy_all_src
from FFSPTrainer import FFSPTrainer as Trainer


##########################################################################################
# parameters

env_params = {
    'stage_cnt': 3,
    'machine_cnt_list': [4, 4, 4],
    'job_cnt': JOB_CNT,
    'process_time_params': {
        'time_low': 2,
        'time_high': 10,
    },
    'reverse': False,
    'pomo_size': POMO_SIZE  # assuming 4 machines at each stage! 4*3*2*1 = 24
}

il_env_params = {
    'stage_cnt': 3,
    'machine_cnt_list': [4, 4, 4],
    'job_cnt': JOB_CNT,
    'process_time_params': {
        'time_low': 2,
        'time_high': 10,
    },
    'reverse': False,
    'pomo_size': 1  # assuming 4 machines at each stage! 4*3*2*1 = 24
}

model_params = {
    'stage_cnt': env_params['stage_cnt'],
    'machine_cnt_list': env_params['machine_cnt_list'],
    'embedding_dim': 256,
    'sqrt_embedding_dim': 256**(1/2),
    'encoder_layer_num': 3,
    'qkv_dim': 16,
    'sqrt_qkv_dim': 16**(1/2),
    'head_num': 16,
    'logit_clipping': 10,
    'ff_hidden_dim': 512,
    'ms_hidden_dim': 16,
    'ms_layer1_init': (1/2)**(1/2),
    'ms_layer2_init': (1/16)**(1/2),
    'eval_type': 'argmax',
    'one_hot_seed_cnt': 4,  # must be >= machine_cnt
}

optimizer_params = {
    'optimizer': {
        'lr': 1e-4,
        'weight_decay': 1e-6
    },
    'scheduler': {
        # 'milestones': [101, 151],
        'milestones': [1000//POMO_SIZE, 1500//POMO_SIZE], 
        # lr step cum_samples 1M and 1.5M (1M / (pomo_size))
        'gamma': 0.1
    }
}

if DISTIL_COEFF == 0.:
    method = 'baseline'
else:
    method = 'ours'
    if NO_SYMMETRIC:
        method += '_nonsym'

trainer_params = {
    'use_cuda': USE_CUDA,
    'cuda_device_num': CUDA_DEVICE_NUM,
    'epochs': 2000//POMO_SIZE + 1,
    'train_episodes': 1*1000,
    'train_batch_size': 50 if JOB_CNT <= 50 else 25,
    'logging': {
        'model_save_interval': 10,
        'img_save_interval': 100,
        'log_image_params_1': {
            'json_foldername': 'log_image_style',
            'filename': 'style.json'
        },
        'log_image_params_2': {
            'json_foldername': 'log_image_style',
            'filename': 'style_loss.json'
        },
    },
    'model_load': {
        'enable': False,  # enable loading pre-trained model
        # 'path': './result/saved_ffsp_model',  # directory path of pre-trained model and log files saved.
        # 'epoch': 100,  # epoch version of pre-trained model to load.
    },
    'wandb': False,
    'run_name': method + '_' + str(POMO_SIZE) + '_' + str(SEED) + '_',
    'pomo_size': POMO_SIZE,  # for logging
    'il_coefficient': DISTIL_COEFF,
    'no_symmetric': NO_SYMMETRIC,
    'seed': SEED,
    'clipping': 10.0
}

tester_params = {
    'use_cuda': USE_CUDA,
    'cuda_device_num': CUDA_DEVICE_NUM,
    # 'model_load': {
    #     'path': './result/saved_ffsp20_model',  # directory path of pre-trained model and log files saved.
    #     'epoch': 100,  # epoch version of pre-trained model to load.
    # },
    'saved_problem_folder': "../data",
    'saved_problem_filename': "unrelated_10000_problems_444_job{}_2_10.pt".format(JOB_CNT),
    'problem_count': 1*1000,
    'test_batch_size': 1000,
    'augmentation_enable': False,
    'aug_factor': 128,
    'aug_batch_size': 200,
}
if tester_params['augmentation_enable']:
    tester_params['test_batch_size'] = tester_params['aug_batch_size']


logger_params = {
    'log_file': {
        'desc': 'matnet_train',
        'filename': 'log.txt'
    }
}


##########################################################################################
# main

def main():
    if DEBUG_MODE:
        _set_debug_mode()

    create_logger(**logger_params)
    _print_config()

    trainer = Trainer(env_params=env_params,
                      model_params=model_params,
                      optimizer_params=optimizer_params,
                      trainer_params=trainer_params,
                      tester_params=tester_params,
                      il_env_params=il_env_params)

    copy_all_src(trainer.result_folder)

    trainer.run()

    if DEBUG_MODE:
        # Print Scehdule for last batch problem
        # env.print_schedule()
        pass


def _set_debug_mode():
    global env_params
    # env_params['job_cnt'] = 5
    # il_env_params['job_cnt'] = 5

    global trainer_params
    trainer_params['epochs'] = 2
    trainer_params['train_episodes'] = 4
    trainer_params['train_batch_size'] = 2
    trainer_params['validate_episodes'] = 4
    trainer_params['validate_batch_size'] = 2
    trainer_params['wandb'] = False

def _print_config():
    logger = logging.getLogger('root')
    logger.info('DEBUG_MODE: {}'.format(DEBUG_MODE))
    logger.info('USE_CUDA: {}, CUDA_DEVICE_NUM: {}'.format(USE_CUDA, CUDA_DEVICE_NUM))
    [logger.info(g_key + "{}".format(globals()[g_key])) for g_key in globals().keys() if g_key.endswith('params')]


##########################################################################################

if __name__ == "__main__":
    main()
